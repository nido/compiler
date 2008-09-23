/* 

  Copyright (C) 2006 ST Microelectronics, Inc.  All Rights Reserved. 

  This program is free software; you can redistribute it and/or modify it 
  under the terms of version 2 of the GNU General Public License as 
  published by the Free Software Foundation. 
  This program is distributed in the hope that it would be useful, but 
  WITHOUT ANY WARRANTY; without even the implied warranty of 
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 

  Further, this software is distributed without any warranty that it is 
  free of the rightful claim of any third person regarding infringement 
  or the like.  Any license provided herein, whether implied or 
  otherwise, applies only to this software file.  Patent licenses, if 
  any, provided herein do not apply to combinations of this program with 
  other software, or any other product whatsoever. 
  You should have received a copy of the GNU General Public License along 
  with this program; if not, write the Free Software Foundation, Inc., 59 
  Temple Place - Suite 330, Boston MA 02111-1307, USA. 

  Contact information:  ST Microelectronics, Inc., 
  , or: 

  http://www.st.com 

  For further information regarding this notice, see: 

  http: 
*/
/*
 * API used by both Front-end and Back-end to load extension dlls
 * and initialize common structures (mtypes, intrinsics)
 */



#include "extension_include.h"
#include "extension_intrinsic.h"
#include "loader.h"
#include "erglob.h"
//TB Extend PREG registers to extension
extern "C"
{
  BE_EXPORTED extern   INT Get_Static_Last_Dedicated_Preg_Offset(void);
  BE_EXPORTED extern   INT Get_Last_Dedicated_Preg_Offset_Func(void);
  BE_EXPORTED extern   void Set_Last_Dedicated_Preg_Offset(INT offset);
}

#ifdef BACK_END
#include "dyn_isa_api.h"
extern "C" { extern const ISA_EXT_Interface_t* get_ISA_extension_instance();};
#endif
#include "errors.h"
#include "config.h"
#include "intrn_info.h"
#include "tracing.h"
#include "dso.h"
#include "mtypes.h"
#include "wn.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include "opcode.h"
#include "lai_loader_api.h"
#include "pixel_mtypes.h"
#include "ext_info.h"
#include "config_TARG.h"

/* Include local helpers. */
#include "attribute_map_template.cxx"

#if defined(__MINGW32__) || defined(__CYGWIN__)
#define SO_EXT ".dll"
#else
#define SO_EXT ".so"
#endif

extern "C" {
#include "W_dlfcn.h"		    /* for sgidladd(), dlerror() */
};
#if defined(BACK_END) 
#include "targ_isa_registers.h"
#include "targ_isa_subset.h"
#endif

// Variables common to gcc and lai loader
// --------------------------------------
// Table to save PREG -> reg class association
static ISA_REGISTER_CLASS *extension_Preg_To_RegClass_table;
// Table to save PREG -> reg number association
int *extension_Preg_To_RegNum_table;
// Table containing Rclass to preg min offset
INT *extension_RegClass_To_Preg_Min_Offset_table;
// Table containing MMODE to MTYPE mapping
static TYPE_ID *Extension_MachineMode_To_Mtype_Array;
static int Extension_MachineMode_To_Mtype_Array_Size=0;

/*
 * Table containing all loaded extension
 */
static Extension_dll_t *extension_tab = (Extension_dll_t*)NULL;
static int extension_count = 0;


#include <map>
typedef std::map<OPCODE, INTRINSIC_Vector_t*> INTRINSIC_OPCODE_MAP_t;
static INTRINSIC_OPCODE_MAP_t Intrinsic_from_OPCODE;

/** 
 * return a vector of intrinsic ids corresponding to opcode.
 * 
 * @param opc 
 * 
 * @return 
 */
extern INTRINSIC_Vector_t* Get_Intrinsic_from_OPCODE(OPCODE opc) {
  if (Intrinsic_from_OPCODE.count(opc)>0)
    return Intrinsic_from_OPCODE[opc];
  return NULL;
}

/** 
 * add the intrinsic idx given in parameter to the
 * Intrinsic_from_OPCODE map
 * 
 * @param opc 
 * @param intrn 
 * 
 */
extern void Add_Intrinsic_for_OPCODE(OPCODE opc, INTRINSIC intrn) {
  if (! Intrinsic_from_OPCODE.count(opc)>0) {
    Intrinsic_from_OPCODE[opc] = new INTRINSIC_Vector_t();
  }
  Intrinsic_from_OPCODE[opc]->push_back(intrn);
}

mUINT8 pixel_size_per_type[MTYPE_MAX_LIMIT+1];

// Information on type equivalence and corresponding conversion builtins
// between C native types and extension
typedef struct {
  TYPE_ID   ctype;
  INTRINSIC intrn_to_ext;
  INTRINSIC intrn_to_c;
  INTRINSIC clr_intrn;
} equiv_type_t;
static equiv_type_t equiv_type_tab[MTYPE_MAX_LIMIT+1];
static INT equiv_type_enabled = 1;

static void Add_Mtype(machine_mode_t mmode, const char *name, 
		      enum mode_class mclass, unsigned short mbitsize, 
		      unsigned char msize, unsigned char munitsize,
                      /* [vcdv] add pixel_size info to mtype */
                      unsigned char pixel_size,
		      unsigned char mwidermode, machine_mode_t innermode, 
		      TYPE_ID mtype, unsigned short alignement) 
{
  /* Update open64 mtype*/
  if (mtype <= MTYPE_COUNT) {
    // MTYPE already exists, nothing to do
    return;
  }
  FmtAssert (mtype < MTYPE_MAX_LIMIT,
	     ("Too many MTYPES. Limit is %d", MTYPE_MAX_LIMIT));

  MTYPE_COUNT++;
  Machine_Types[MTYPE_COUNT].id = mtype;
  Machine_Types[MTYPE_COUNT].bit_size = mbitsize;
  Machine_Types[MTYPE_COUNT].byte_size = msize;
  Machine_Types[MTYPE_COUNT].dummy1 = msize;
  Machine_Types[MTYPE_COUNT].alignment = alignement;
  Machine_Types[MTYPE_COUNT].dummy2 = 0;
  Machine_Types[MTYPE_COUNT].dummy3 = 0;
  Machine_Types[MTYPE_COUNT].signed_type = 0;
  Machine_Types[MTYPE_COUNT].float_type = 0;
  Machine_Types[MTYPE_COUNT].dummy4 = 0;
  Machine_Types[MTYPE_COUNT].name = (char*)name;

  /* [vcdv] add pixel_size info to mtype */
  if (MTYPE_COUNT>MTYPE_STATIC_LAST)
    MTYPE_pixel_size(MTYPE_COUNT) = pixel_size;
   
  // MMODE=>MTYPE mapping
  // Set the associated MTYPE
  if (mmode != -1)
    Extension_MachineMode_To_Mtype_Array[mmode - STATIC_COUNT_MACHINE_MODE] = mtype;
  mUINT8 type_class = 0;
  switch (mclass) { 
  case  MODE_RANDOM:
    type_class |= MTYPE_CLASS_UNKNOWN;
    break;
  case MODE_INT:
    type_class |= MTYPE_CLASS_INTEGER;
    break;
  case MODE_FLOAT:
    type_class |= MTYPE_CLASS_FLOAT;
    break;
  case MODE_COMPLEX_INT:
    type_class |= MTYPE_CLASS_COMPLEX | MTYPE_CLASS_INTEGER;
    break;
  case MODE_COMPLEX_FLOAT:
    type_class |= MTYPE_CLASS_COMPLEX | MTYPE_CLASS_FLOAT;
    break;
  case MODE_VECTOR_INT:
    type_class |= MTYPE_CLASS_RANDOM;
    //    type_class |= MTYPE_CLASS_VECTOR | MTYPE_CLASS_INTEGER;
    break;
  case MODE_VECTOR_FLOAT:
    type_class |= MTYPE_CLASS_RANDOM;
    //type_class |= MTYPE_CLASS_VECTOR | MTYPE_CLASS_FLOAT;
    break;
  case MODE_PARTIAL_INT:
  case MODE_CC:
  case MAX_MODE_CLASS:
  default:
    FmtAssert (FALSE,
	       ("Unknown translation between GCC mode class and OPEN64 MTYPE class %d", mclass));
    break;
  }
  Machine_Types[MTYPE_COUNT].type_class = type_class;
  Machine_Types[MTYPE_COUNT].type_order = mbitsize;
  Machine_Types[MTYPE_COUNT].complement = mtype;
}


/*
 * Extend MType list with all mtype defined by the specified dll
 * instance.  Returns the base MTYPE for this extension and a mapping
 * between mtypes and local register class for this extension.
 */
TYPE_ID Add_MTypes(const Extension_dll_t *dll_instance,
		   int **mtype_to_locrclass,
		   int *mtype_count,
		   BOOL verbose) {
  unsigned int i;
  unsigned int count = dll_instance->hooks->get_modes_count();
  const extension_machine_types_t* modes = dll_instance->hooks->get_modes();
  //Local mapping between mtype and local rclass, needed bu the front
  //end to get the symbol associated to a PREG
  int *mtype_to_local_rclass;
  if (mtype_to_locrclass)
    mtype_to_local_rclass = TYPE_MEM_POOL_ALLOC_N(int, Malloc_Mem_Pool, count);
  TYPE_ID base_mtype = MTYPE_COUNT;
  for (i = 0; i < count; i++) {
    TYPE_ID new_mtype = DLL_TO_GLOBAL_MTYPE(dll_instance, modes[i].mtype);
    machine_mode_t new_mmode = DLL_TO_GLOBAL_MMODE(dll_instance, modes[i].mmode);
    
    if (verbose) {
      fprintf(TFile, "  Extension '%s': Associate dynamic Mtype '%s'"
	      " to Mtype id %d (machine mode: %d)\n",
	      dll_instance->extname, modes[i].name, new_mtype, new_mmode);
    }
    Add_Mtype(new_mmode, 
	      modes[i].name, 
	      modes[i].mclass, 
	      modes[i].mbitsize,
	      modes[i].msize,
	      modes[i].munitsize,
#ifdef TARG_ST
              /* [vcdv] add pixel_size info to mtype */
              modes[i].mpixelsize,
#endif
	      modes[i].mwidermode, 
	      modes[i].innermode,
	      new_mtype,
	      modes[i].alignment);
    // Here, the new MTYPE has the id MTYPE_COUNT
    // Keep a mapping between this MTYPE and its local register file
    if (mtype_to_locrclass)
      mtype_to_local_rclass[i] = modes[i].local_REGISTER_CLASS_id;
  }
  /* [VCdV] reset pixel_size to zero for all static types */
  for (i=0; i<=MTYPE_STATIC_LAST; i++) {
    MTYPE_pixel_size(i)=0;
  }
  
  if (mtype_to_locrclass)
    *mtype_to_locrclass = mtype_to_local_rclass;
  if (mtype_count)
    *mtype_count = count;
  return base_mtype + 1;
}

/*
 * Add a single intrinsic to the intrinsic list
 */
static void
Add_Intrinsic(const Extension_dll_t *dll_instance,
	      const extension_builtins_t* btypes,
	      BOOL verbose) {
  INTRINSIC intrn_id = INTRINSIC_COUNT + 1;
  
  intrn_info[intrn_id].is_by_val           = btypes->is_by_val;
  intrn_info[intrn_id].is_pure             = btypes->is_pure;
  intrn_info[intrn_id].has_no_side_effects = btypes->has_no_side_effects;
  intrn_info[intrn_id].never_returns       = btypes->never_returns;
  intrn_info[intrn_id].is_actual           = btypes->is_actual;
  intrn_info[intrn_id].is_cg_intrinsic     = btypes->is_cg_intrinsic;
  intrn_info[intrn_id].c_name              = (char*)btypes->c_name;
  intrn_info[intrn_id].specific_name       = (char*)NULL;
  intrn_info[intrn_id].runtime_name        = (char*)btypes->runtime_name;


  EXTENSION_add_INTRINSIC_to_Map(intrn_info[intrn_id].runtime_name, intrn_id);

  // Complete proto info for this intrinsic
  proto_intrn_info_t *proto_info =
    &Proto_Intrn_Info_Array[INTRINSIC_COUNT - INTRINSIC_STATIC_COUNT];
  proto_info->argument_count = btypes->arg_count;
  
  FmtAssert(btypes->arg_count <= INTRN_MAX_ARG,
	    ("Intrinsic %s exceeds the limit of supported number of parameters (%d)\n.",
	     btypes->c_name, INTRN_MAX_ARG));
  // Add Mtype info: Convert machine mode proto info coming from the
  // dll into MTYPE. We need this info only for out and inout
  // parameters.
  TYPE_ID outmtype[INTRN_MAX_ARG];
  for (int arg = 0; arg < btypes->arg_count; arg++) {
    outmtype[arg] = MachineMode_To_Mtype(btypes->arg_type[arg]);
  }
  INTRN_INOUT_TYPE inouttype[INTRN_MAX_ARG];
  int nb_out = 0;
  int nb_in = 0;
  for (int arg = 0; arg < btypes->arg_count; arg++) {
    FmtAssert(btypes->arg_inout[arg] != BUILTARG_UNDEF,
	      ("Undef parameter type for intrinsic %s\n.", btypes->c_name));
    if (btypes->arg_inout[arg] == BUILTARG_OUT || btypes->arg_inout[arg] == BUILTARG_INOUT)
      nb_out++;
    if (btypes->arg_inout[arg] == BUILTARG_IN || btypes->arg_inout[arg] == BUILTARG_INOUT)
      nb_in++;
    // enum INTRN_INOUT_TYPE and enum BUILTARG_INOUT_TYPE are in the
    // same order, so just cast to map.
    inouttype[arg] = (INTRN_INOUT_TYPE)btypes->arg_inout[arg];
  }
  
  proto_info->arg_type = TYPE_MEM_POOL_ALLOC_N(TYPE_ID, Malloc_Mem_Pool, btypes->arg_count);
  memcpy(proto_info->arg_type, outmtype, btypes->arg_count*sizeof(TYPE_ID));
  proto_info->arg_inout =  TYPE_MEM_POOL_ALLOC_N(INTRN_INOUT_TYPE, Malloc_Mem_Pool, btypes->arg_count);
  memcpy(proto_info->arg_inout, inouttype, btypes->arg_count*sizeof(TYPE_ID));
  
  proto_info->argument_count = btypes->arg_count;
  proto_info->arg_out_count  = nb_out;
  proto_info->arg_in_count   = nb_in;
    
  if (btypes->return_type != VOIDmode && (nb_out != 0)) {
    char err_msg[256];
    sprintf(err_msg,
	    "Incompatible prototype for built-in %s. "
	    "A built-in cannot have out parameters and not returning void.",
	    btypes->c_name);
    RaiseErrorIncompatibleLibrary(dll_instance->dllname, err_msg);
  }
  
  // Initialize returned type
  proto_info->return_type = MachineMode_To_Mtype(btypes->return_type);
  intrn_info[intrn_id].return_kind = INTRN_return_kind_for_mtype(MachineMode_To_Mtype(btypes->return_type));

  // Build tables used for code generation
  {
    if (is_DYN_INTRN_CLR(*btypes)) {
      TYPE_ID ext_ty =  MachineMode_To_Mtype(btypes->return_type);
      equiv_type_tab[ext_ty].clr_intrn = intrn_id;
    }
    
    if (is_DYN_INTRN_CONVERT_TO_CTYPE(*btypes)) {
    TYPE_ID ext_ty = MachineMode_To_Mtype(btypes->arg_type[0]);
    TYPE_ID c_ty   = MachineMode_To_Mtype(btypes->return_type);
    equiv_type_tab[ext_ty].ctype = MTYPE_is_signed(c_ty)?MTYPE_complement(c_ty):c_ty; // Register unsigned type
    equiv_type_tab[ext_ty].intrn_to_c = intrn_id;
    }
    else if (is_DYN_INTRN_CONVERT_FROM_CTYPE(*btypes)) {
      TYPE_ID c_ty   = MachineMode_To_Mtype(btypes->arg_type[0]);
      TYPE_ID ext_ty = MachineMode_To_Mtype(btypes->return_type);
      equiv_type_tab[ext_ty].ctype = MTYPE_is_signed(c_ty)?MTYPE_complement(c_ty):c_ty; // Register unsigned type
      equiv_type_tab[ext_ty].intrn_to_ext = intrn_id;
    }
    
    if (btypes->wn_table != NULL)  {
      int k=0;
      while ( (OPCODE)(btypes->wn_table[k].wn_opc) != OPCODE_UNKNOWN ) {
	OPCODE opc =  (OPCODE)(btypes->wn_table[k].wn_opc);
	Add_Intrinsic_for_OPCODE(opc, intrn_id);
	k++;
      }
    }
  }

  if (verbose) {
    fprintf(TFile, "    intrinsic[%d] = %s\n",
	    (intrn_id), (char*)btypes->runtime_name);
  }
  INTRINSIC_COUNT++;
}

/*
 * Extend intrinsic list with all intrinsics defined by the
 * specified dll instance
 */
void Add_Intrinsics(const Extension_dll_t *dll_instance, BOOL verbose)
{
  unsigned int i;
  unsigned int count = dll_instance->hooks->get_builtins_count();
  const extension_builtins_t* btypes = dll_instance->hooks->get_builtins();
  if (verbose) {
    fprintf(TFile, "  Extension '%s': Adding %d intrinsics starting at id %d\n",
	    dll_instance->extname, count, INTRINSIC_COUNT + 1);
  }
  for (i = 0; i < count; i++) {
    Add_Intrinsic(dll_instance, &btypes[i], verbose);
  }
}

//TB: Add a new pass to create new MTYPE for mutiple result intrinsics
void Add_Composed_Mtype() {
  //Run thru added intrinsics to create new composed mtype
  for (INTRINSIC intr = INTRINSIC_STATIC_COUNT+1; intr <= INTRINSIC_COUNT; intr++)
    {
      char *newmtype_name = TYPE_MEM_POOL_ALLOC_N(char, Malloc_Mem_Pool,
						  1024);
      newmtype_name[0]='.';
      newmtype_name[1]='\0';
      proto_intrn_info_t *proto = INTRN_proto_info(intr);
      //proto->return_type has already been initialized in
      //add_intrinsics For multiple results intrinsics, we update
      //return_type with the newly created composed MTYPE
      if (INTRN_number_of_out_param(proto) <= 1) {
	for (int arg = 0; arg < proto->argument_count; arg ++)
	  if (INTRN_is_out_param(arg, proto)) {
	    proto->return_type = proto->arg_type[arg];
	    //Set return kind
	    intrn_info[intr].return_kind = INTRN_return_kind_for_mtype(proto->return_type);
	  }
	continue;
      }
      for (int i = 0; i < proto->argument_count; i++) {
	if (INTRN_is_out_param(i, proto)) {
	  strcat(newmtype_name, MTYPE_name(proto->arg_type[i]));
	  strcat(newmtype_name, ".");
	}
      }
      //Add a new MTYPE (MTYPE_COUNT +1)
      proto->return_type = MTYPE_COUNT+1;
      Add_Mtype(/*machine_mode_t*/ -1 , newmtype_name, 
		/*enum mode_class*/ MODE_RANDOM, /* mbitsize*/ 0, 
		/*msize*/ 0,  /*munitsize*/ 0,
		/*mwidermode*/ 0,
#ifdef TARG_ST
                /* [vcdv] add pixel_size info to mtype */
                /*mpixelsize*/ 0,
#endif
                /*innermode*/-1, 
		/* mtype*/ MTYPE_COUNT+1, /*alignement*/TARG_NONE_ALIGN);

      //Set return kind
      intrn_info[intr].return_kind = INTRN_return_kind_for_mtype(proto->return_type);
    }
}


/*
 * OPEN64 Mtype init
 */
void Init_Mtypes(int nb_mtype_to_add) {
  MTYPE_COUNT = MTYPE_STATIC_LAST;
  FIRST_COMPOSED_MTYPE = MTYPE_COUNT + nb_mtype_to_add + 1;
  
  if (MTYPE_STATIC_LAST + nb_mtype_to_add > MTYPE_MAX_LIMIT) {
    FmtAssert(FALSE,
	      ("Too much dynamic MTYPEs to add. Compilation aborted."));
  }
  if (nb_mtype_to_add) {
    //TB: Allocate mtype to mmode mapping for extension
    Extension_MachineMode_To_Mtype_Array = TYPE_MEM_POOL_ALLOC_N(TYPE_ID, Malloc_Mem_Pool, nb_mtype_to_add);
    Extension_MachineMode_To_Mtype_Array_Size = nb_mtype_to_add;
  }
}


/*
 * OPEN64 Intrinsics init
 */
void Init_Intrinsics(int nb_builtins_to_add)
{
  INTRINSIC_COUNT = INTRINSIC_STATIC_COUNT;
  if (nb_builtins_to_add == 0) {
    // Nothing to allocate, juste reference static tables
    intrn_info = (intrn_info_t *)intrn_info_static;
  }else {
    intrn_info = TYPE_MEM_POOL_ALLOC_N(intrn_info_t,
				       Malloc_Mem_Pool,
				       (INTRINSIC_COUNT + nb_builtins_to_add + 1));
    memcpy(intrn_info, intrn_info_static, (INTRINSIC_STATIC_COUNT+1)*sizeof(intrn_info_t));
    Proto_Intrn_Info_Array = TYPE_MEM_POOL_ALLOC_N(proto_intrn_info_t,
						   Malloc_Mem_Pool,
						   nb_builtins_to_add);
  }
}


static void
extension_dll_init( Extension_dll_t *ext_inter, 
                    char *path, BOOL verbose )
{
  FmtAssert((ext_inter->dllname!=NULL),
	    ("NULL name for extension\n"));
  
  ext_inter->handler = load_so_no_RTLD_GLOBAL(ext_inter->dllname,path,verbose);
  return;
}

static void
extension_dll_close( Extension_dll_t *ext )
{
  close_so(ext->handler);
  ext->handler=NULL;
  if(ext->dllname) {
    free(ext->dllname);
    ext->dllname = (char*)NULL;
  }
  if(ext->extname) {
    free(ext->extname);
    ext->extname = (char*)NULL;
  }
  return;
}


/*
 * Retrieve a symbol from argument dll and return a pointer to it
 * (NULL if undefined)
 */
static void *
Get_dll_Symbol(const Extension_dll_t *dll_instance,
	       const char *symbol) {
  void *symb_ptr;

  FmtAssert((symbol != NULL),
	    ("Unexpected NULL string for symbol name in Get_dll_Symbol()"));

  symb_ptr = dlsym(dll_instance->handler, symbol);
  if(symb_ptr == NULL) {
    char err_msg[256];
    sprintf(err_msg,
	    "Unable to find symbol %s (%s)", symbol, dlerror());
    RaiseErrorIncompatibleLibrary(dll_instance->dllname, err_msg);
  }

  return (symb_ptr);
}


/*
 * Load the extension dll specified by its basename and access path.
 */
static Extension_dll_t
Load_Extension_dll(const char *basename,
		   char *path,
		   BOOL verbose) {
  Extension_dll_t ext_dll;
  static const char *const str_suffix = SO_EXT;
  
  /* Build dll name */
  ext_dll.dllname = TYPE_MEM_POOL_ALLOC_N(char, Malloc_Mem_Pool,
					  strlen(basename)   + 
					  strlen(str_suffix) +
					  1 );
  FmtAssert((ext_dll.dllname!=NULL),
	    ("Unable to allocate memory\n"));
  
  strcpy(ext_dll.dllname, basename);
  strcat(ext_dll.dllname, str_suffix);

  /* Connect dll */
  extension_dll_init(&ext_dll, path, verbose);
    
  if(verbose) {
    fprintf(TFile, "Connecting extension dll %s\n",ext_dll.dllname);
  }
  
  /* Retrieve machine type and builtin interface */
  get_type_extension_instance_t get_instance;
  get_instance = (get_type_extension_instance_t)Get_dll_Symbol(&ext_dll, "get_type_extension_instance");
  const extension_hooks_t *hooks = (*get_instance)();
  if (hooks == NULL) {
    FmtAssert((FALSE),
	      ("Symbol 'get_type_extension_instance' not found in extension dll '%s'\n",
	       ext_dll.dllname));
  }

  // Check extension hooks compatibility and initialize the access API
  INT magic = hooks->magic;
  if (EXTENSION_Is_Supported_HighLevel_Revision(magic)) {
    if (verbose) {
      fprintf(TFile, "  HL API revision: lib=%d, compiler=%d\n", magic, MAGIC_NUMBER_EXT_API);
    }
    ext_dll.hooks = new EXTENSION_HighLevel_Info(hooks);
  }
  else {
    char err_msg[256];
    sprintf(err_msg,
	    "Incompatible HL API revision: lib=%d, compiler=%d.\n",
	    magic, MAGIC_NUMBER_EXT_API);
    RaiseErrorIncompatibleLibrary(ext_dll.dllname, err_msg);
  }

  /* Retrieve extension name */
  get_extension_name_t get_name;
  const char *extname_in_dll;
  get_name = (get_extension_name_t)Get_dll_Symbol(&ext_dll, "get_extension_name");
  extname_in_dll = (*get_name)();
  ext_dll.extname = TYPE_MEM_POOL_ALLOC_N(char, Malloc_Mem_Pool, strlen(extname_in_dll)+1);
  strcpy(ext_dll.extname, extname_in_dll);

  return (ext_dll);
}


/*
 * Load extension dlls and return a pointer to the extension table
 * and the count of loaded extensions
 */
void
Load_Extension_dlls( Extension_dll_t **ext_tab, int *ext_count, BOOL verbose )
{
  int i;
  int nb_ext_mtypes;
  int nb_ext_intrinsics;
  int base_mtypes;
  int base_mmodes;
  int base_builtins;
  int base_intrinsics;
  char *ext_fullpath_ptr;
  char *next_ext_fullpath_ptr;

  /* Maximum number of extensions is 8 */
  *ext_tab = (Extension_dll_t*)calloc(sizeof(Extension_dll_t),8);
  FmtAssert((*ext_tab!=NULL),
	    ("Unable to allocate memory\n"));


  // Load extension dlls, compute base offsets for each element
  // (machine modes, mtypes, intrinsics and builtins)  of  each
  // extension, and count additionnal mtypes and intrinsics.
  base_mtypes       = MTYPE_STATIC_LAST + 1;
  base_mmodes       = MACHINE_MODE_STATIC_LAST + 1;
  base_builtins     = BUILT_IN_STATIC_LAST + 1;
  base_intrinsics   = INTRINSIC_STATIC_LAST + 1;
  ext_fullpath_ptr  = Extension_Names;
  *ext_count        = 0;
  while (NULL!=ext_fullpath_ptr) {
    next_ext_fullpath_ptr = strchr(ext_fullpath_ptr,',');
    if (next_ext_fullpath_ptr) *next_ext_fullpath_ptr++=0;

    (*ext_tab)[*ext_count] = Load_Extension_dll(ext_fullpath_ptr,"",verbose);
    (*ext_tab)[*ext_count].base_mtypes     = base_mtypes;
    (*ext_tab)[*ext_count].base_mmodes     = base_mmodes;
    (*ext_tab)[*ext_count].base_builtins   = base_builtins;
    (*ext_tab)[*ext_count].base_intrinsics = base_intrinsics;

    // Check that extension has not yet been loaded
    for (i=0; i<(*ext_count); i++) {
      if (!strcmp((*ext_tab)[*ext_count].extname, (*ext_tab)[i].extname)) {
	break;
      }
    }
    if (i < (*ext_count)) { // Extension already loaded
      DevWarn("Extension '%s' specified several times in command line (dll '%s' and '%s')! First one loaded only.\n",
	      (*ext_tab)[*ext_count].extname, (*ext_tab)[i].dllname, (*ext_tab)[*ext_count].dllname);
      extension_dll_close(&(*ext_tab)[*ext_count]);
      ext_fullpath_ptr = next_ext_fullpath_ptr;
      continue;
    }

    nb_ext_mtypes     = (*ext_tab)[*ext_count].hooks->get_modes_count();
    nb_ext_intrinsics = (*ext_tab)[*ext_count].hooks->get_builtins_count();
    
    base_mtypes     += nb_ext_mtypes;
    base_mmodes     += nb_ext_mtypes;
    base_builtins   += nb_ext_intrinsics;
    base_intrinsics += nb_ext_intrinsics;
    (*ext_count)++;
    ext_fullpath_ptr = next_ext_fullpath_ptr;
  }
  
}


#if defined(BACK_END) || defined(IR_TOOLS)
/*
 * Main function responsible for loading the extension library and
 * initializing dynamic mtypes and intrinsics
 */
void
Initialize_Extension_Loader ()
{
  int i;
  int nb_ext_mtypes;
  int nb_ext_intrinsics;

  CGTARG_InitializeMetaInstructionMap();
  

  for (i=0; i<=MTYPE_MAX_LIMIT; i++) {
    equiv_type_tab[i].ctype        = MTYPE_UNKNOWN;
    equiv_type_tab[i].intrn_to_c   = INTRINSIC_INVALID;
    equiv_type_tab[i].intrn_to_ext = INTRINSIC_INVALID;
    equiv_type_tab[i].clr_intrn    = INTRINSIC_INVALID;
  }

  if (Extension_Is_Present) {
 
    BOOL verbose = Get_Trace(TP_EXTENSION, TRACE_EXTENSION_LOADER_MASK);
    
    Load_Extension_dlls(&extension_tab, &extension_count, verbose);

    // Compute total count for mtypes and intrinsics
    nb_ext_mtypes     = 0;
    nb_ext_intrinsics = 0;
    for (i=0; i<extension_count; i++) {
      nb_ext_mtypes     += extension_tab[i].hooks->get_modes_count();
      nb_ext_intrinsics += extension_tab[i].hooks->get_builtins_count();
    }

    Init_Mtypes(nb_ext_mtypes);
    Init_Intrinsics(nb_ext_intrinsics);

    for (i=0; i<extension_count; i++) {
      Add_MTypes(&extension_tab[i], NULL, NULL, verbose);
      Add_Intrinsics(&extension_tab[i], verbose);
    }
    //TB: Add a new pass to create new MTYPE for mutiple result intrinsics
    Add_Composed_Mtype();

    /* (Partially) Disable extension native support when:
     * - Compiling at optimization level < 2
     * - No opcode->intrinsic correspondence found (useless walk)
     */
    if (Opt_Level < 2 && !Enable_Extension_Native_Support_Set) {
      Enable_Extension_Native_Support&= ~(EXTENSION_NATIVE_CODEGEN |
					  EXTENSION_NATIVE_CVTGEN);
    }
    else if (Intrinsic_from_OPCODE.empty()) {
      Enable_Extension_Native_Support&= ~EXTENSION_NATIVE_CODEGEN;
    }
  }
  else {  // !Extension_Is_Present
    Init_Mtypes(0);
    Init_Intrinsics(0);
  }
}

/*
 * This function is responsible for setting PREG info To be called by
 * the inliner or ir_b2a (not the front end neither the back end)
 */
void
Initialize_Extension_Loader_Register () {
  int i;
  int nb_ext_mtypes;
  int nb_ext_intrinsics;

  if (Extension_Is_Present) {
    for (i=0; i < extension_count; i++) {
      int j;
      EXTENSION_ISA_Info* isa_ext_access;
      const ISA_REGISTER_CLASS_INFO* rc_tab;
      isa_ext_access = Generate_EXTENSION_ISA_Info(&extension_tab[i], 0);
      rc_tab = isa_ext_access->get_ISA_REGISTER_CLASS_tab();
      const mUINT32 rc_tab_size = isa_ext_access->get_ISA_REGISTER_CLASS_tab_sz();
      for (j=0; j < rc_tab_size; j++) {
	int first_isa_reg  = rc_tab[j].min_regnum;
	int last_isa_reg   = rc_tab[j].max_regnum;
	int register_count = last_isa_reg - first_isa_reg + 1;
	// Set the PREG max bound
	Set_Last_Dedicated_Preg_Offset( Get_Last_Dedicated_Preg_Offset_Func() + register_count);
      }
      delete isa_ext_access;
    }
  }
}

/*
 * Initialize PU specific options
 */
void Initialize_Extension_Loader_PU(WN *pu) {
  /*
   * Enable/disable extension type equivalences on a function basis,
   * based on both Enable_Extension_Native_Support value and pragma
   * value
   */
  INT type_equiv_enabled = (Enable_Extension_Native_Support != 0);
  if (WN_operator(pu) == OPR_FUNC_ENTRY && WN_func_pragmas(pu)) {
    WN_PRAGMA_ID invert_id;
    if (type_equiv_enabled) {
      invert_id = WN_PRAGMA_DISABLE_EXTGEN;
    } else {
      invert_id = WN_PRAGMA_FORCE_EXTGEN;
    }
    WN *wn;
    for (wn = WN_first(WN_func_pragmas(pu)); wn; wn = WN_next(wn)) {
      if (((WN_opcode(wn) == OPC_PRAGMA) || (WN_opcode(wn) == OPC_XPRAGMA))
	  && ((WN_PRAGMA_ID)WN_pragma(wn) == invert_id)) {
	type_equiv_enabled = !type_equiv_enabled;
	break;
      }
    }
  }
  EXTENSION_Set_Equivalent_Mtype_Status(type_equiv_enabled);
}
#endif
#ifdef BACK_END
extern "C" {
  void TI_Initialize_Extension_Loader() {
    if (Extension_Is_Present) {
      Lai_Initialize_Extension_Loader (extension_count, extension_tab);
    }
  }
}
#endif

void Cleanup_Extension_Loader() {
  int i;
  for (i=0; i<extension_count; i++) {
    extension_dll_close(&extension_tab[i]);
  }
  free(extension_tab);
}


/* ===========================================================================
 * Initialize ISA REgister classes tables from statically defined ones
 * concatenated with extension defined ones.
 * Initialized data are: 
 * - ISA_REGISTER_CLASS_info        (array [ISA_REGISTER_CLASS_MAX+1])
 *                                              UNDEFINED at offset 0
 * - ISA_REGISTER_CLASS_info_index  (array [ISA_REGISTER_CLASS_MAX+1])
 * - ISA_REGISTER_SUBCLASS_info     (array [ISA_REGISTER_SUBCLASS_MAX+1])
 * ===========================================================================*/
void Initialize_ISA_RegisterClasses(Lai_Loader_Info_t &ext_info) {
  int ext, j;
  int static_size;
  ISA_REGISTER_CLASS_INFO    *rc_tab;
  ISA_REGISTER_SUBCLASS_INFO *subrc_tab;
  mUINT8 *rc_index_tab;

  if (ext_info.trace_on) {
    fprintf(TFile, "...Initialize extension ISA Register classes\n");
  }

  int nb_added_rclass = ISA_REGISTER_CLASS_MAX - ISA_REGISTER_CLASS_STATIC_MAX;
  int nb_added_rsubclass = ISA_REGISTER_SUBCLASS_MAX - ISA_REGISTER_SUBCLASS_STATIC_MAX;
  int nb_added_preg = Get_Last_Dedicated_Preg_Offset_Func() - Get_Static_Last_Dedicated_Preg_Offset();

  if (nb_added_rclass == 0) {
    return;
  }

  // Resize register class table
  static_size = ISA_REGISTER_CLASS_STATIC_MAX + 1;
  rc_tab = TYPE_MEM_POOL_ALLOC_N(ISA_REGISTER_CLASS_INFO, Malloc_Mem_Pool,
				 (static_size + nb_added_rclass));
  memcpy(rc_tab, ISA_REGISTER_CLASS_info, static_size * sizeof(ISA_REGISTER_CLASS_INFO));
  
  extension_Preg_To_RegClass_table =  TYPE_MEM_POOL_ALLOC_N(ISA_REGISTER_CLASS,
						      Malloc_Mem_Pool,
						      nb_added_preg);
  extension_Preg_To_RegNum_table =  TYPE_MEM_POOL_ALLOC_N(int,
							  Malloc_Mem_Pool,
							  nb_added_preg);
  //TB: add a mapping between register class and Preg_Min_Offset for this class
  extension_RegClass_To_Preg_Min_Offset_table = TYPE_MEM_POOL_ALLOC_N(INT,
								      Malloc_Mem_Pool,
								      nb_added_rclass);
  // Resize register class index table
  // (it might be oversized, as several entries in register class table might
  //  represent different regset of the same real register class)
  rc_index_tab = TYPE_MEM_POOL_ALLOC_N(mUINT8, Malloc_Mem_Pool,
				       static_size + nb_added_rclass);
  memcpy(rc_index_tab, ISA_REGISTER_CLASS_info_index, static_size * sizeof(mUINT8));
  
  // Resize sub register class table
  if (nb_added_rsubclass > 0) {
    static_size   = ISA_REGISTER_SUBCLASS_STATIC_MAX + 1;
    subrc_tab = TYPE_MEM_POOL_ALLOC_N(ISA_REGISTER_SUBCLASS_INFO,
				      Malloc_Mem_Pool,
				      static_size + nb_added_rsubclass);
    memcpy(subrc_tab, ISA_REGISTER_SUBCLASS_info,
	   static_size * sizeof(ISA_REGISTER_SUBCLASS_INFO));
  }

  int  next_rc_index  = ISA_REGISTER_CLASS_INDEX_DYNAMIC_NEXT; // next value to be put in REGISTER_CLASS_index table
  int  next_rc_offset = ISA_REGISTER_CLASS_STATIC_MAX+1; // next REGISTER_CLASS_index table offset to fill
  int Local_Last_Dedicated_Preg_Offset = Get_Static_Last_Dedicated_Preg_Offset();
  for (ext=0; ext<ext_info.nb_ext; ext++) {
    int rc_in_ext    = ext_info.ISA_tab[ext]->get_ISA_REGISTER_CLASS_tab_sz();
    int subrc_in_ext = (nb_added_rsubclass > 0) ? ext_info.ISA_tab[ext]->get_ISA_REGISTER_SUBCLASS_tab_sz() : 0;
    FmtAssert((Local_Last_Dedicated_Preg_Offset + 1 == ext_info.base_PREG[ext]),("Difference between base preg and conputed one for extension %d", ext));
    if (rc_in_ext > 0) {
      
      // Add register classes
      memcpy(&rc_tab[ext_info.base_REGISTER_CLASS[ext]],
	     ext_info.ISA_tab[ext]->get_ISA_REGISTER_CLASS_tab(),
	     rc_in_ext * sizeof(ISA_REGISTER_CLASS_INFO));

      /* Backward compatibility. We ignore the subset mask (isa_mask) for the register class.
	 Actually, we force it to be the principal currently active subset, such that the register
	 calss is always activated.
	 Anyway any loaded register class is active as soon as the extensiojn is active, 
	 i.e. this information is not used.
	 For instance on stxp70, the stxp70 RTK may generate either the mask (1<<ISA_SUBSET_stxp70) or
	 the mask (1<<ISA_SUBSET_stxp70_ext) while ISA_SUBSET_stxp70_ext is now obsolete. */
      rc_tab[ext_info.base_REGISTER_CLASS[ext]].isa_mask = (1<<*(ISA_SUBSET_LIST_Begin(ISA_SUBSET_List)));

      //TB: Update PREG with regclass
      for (int regcl = ext_info.base_REGISTER_CLASS[ext]; 
	   regcl < ext_info.base_REGISTER_CLASS[ext] + rc_in_ext; 
	   regcl++) {
	int first_isa_reg  = rc_tab[regcl].min_regnum;
	int last_isa_reg   = rc_tab[regcl].max_regnum;
	int register_count = last_isa_reg - first_isa_reg + 1;
	for (int k = Local_Last_Dedicated_Preg_Offset; 
	     k < Local_Last_Dedicated_Preg_Offset + register_count; 
	     k++) {
	  // Update PREG to register class array
	  extension_Preg_To_RegClass_table[k - Get_Static_Last_Dedicated_Preg_Offset() ] = regcl; 
	  extension_Preg_To_RegNum_table[k - Get_Static_Last_Dedicated_Preg_Offset() ] = first_isa_reg + (k - Local_Last_Dedicated_Preg_Offset) ; 
	}
	extension_RegClass_To_Preg_Min_Offset_table[regcl - ISA_REGISTER_CLASS_STATIC_MAX - 1] = Local_Last_Dedicated_Preg_Offset + 1;
	Local_Last_Dedicated_Preg_Offset += register_count;
      }
      // Add register subclasses
      if (subrc_in_ext > 0) {
	memcpy(&subrc_tab[ext_info.base_REGISTER_SUBCLASS[ext]],
	       ext_info.ISA_tab[ext]->get_ISA_REGISTER_SUBCLASS_tab(),
	       subrc_in_ext * sizeof(ISA_REGISTER_SUBCLASS_INFO));

	// ...Update reference to the register class
	for (j=0; j<subrc_in_ext; j++) {
	  subrc_tab[ext_info.base_REGISTER_SUBCLASS[ext]+j].rclass += ext_info.base_REGISTER_CLASS[ext];
	}
      }

      for (j=0; j<rc_in_ext; j++) {
	// Fill index table (one regclass can have several regsets)
	// TODO: if several entries of the ISA_REGISTER_CLASS_info table are
	//       associated to the same register class name,
	//       then ISA_REGISTER_CLASS_MAX should be decremented to reflect that.
	//   --> Fortunately, a check is done in isa_register_gen to forbid
	//       register class with multiple register sets.
	if ((j==0) ||
	    (strcmp(rc_tab[ext_info.base_REGISTER_CLASS[ext]+j-1].name,
		    rc_tab[ext_info.base_REGISTER_CLASS[ext]+j].name) != 0)) {
	  rc_index_tab[next_rc_offset++] = next_rc_index;
	}
	next_rc_index++;
      }
    }
  }

  // Update targinfo pointers
  ISA_REGISTER_CLASS_info       = rc_tab;
  if (nb_added_rsubclass > 0) {
    ISA_REGISTER_SUBCLASS_info  = subrc_tab;
  }
  ISA_REGISTER_CLASS_info_index = rc_index_tab;

  FmtAssert((Local_Last_Dedicated_Preg_Offset == Get_Last_Dedicated_Preg_Offset_Func()),
	    ("Computed max preg != Last_Dedicated_Preg_Offset"));
}

/* ===========================================================================
 * Initialize ABI PROPERTIES tables from statically defined ones concatenated
 * with extension defined ones. Initialized data are: 
 * - ABI_PROPERTIES_base_props     (array)
 *
 * ===========================================================================*/
void Initialize_ABI_Properties(Lai_Loader_Info_t &ext_info) {
  int ext;
  ABI_PROPERTIES *abi;
  int             abi_idx;
  
  if (ext_info.trace_on) {
    fprintf(TFile, "...Initialize extension ABI properties\n");
  }

  if (ISA_REGISTER_CLASS_MAX == ISA_REGISTER_CLASS_STATIC_MAX) {
    return;  // No extension register classes
  }

  // Allocate new table
  abi = TYPE_MEM_POOL_ALLOC_N(ABI_PROPERTIES, Malloc_Mem_Pool,
			      (ABI_PROPERTIES_ABI_MAX+1));

  // For each ABI, allocate new tables, initialize them with the content
  // of the initial static tables and complete them extension specific
  // data
  // Warning, the model which has been adopted so far consider
  // that we define only one ABI extension for each extension.
  // As a result, we always have only one item in extension
  // ABI_PROPERTIES table for extension.
  for (abi_idx=0; abi_idx<=ABI_PROPERTIES_ABI_MAX; abi_idx++) {
    mUINT32     **reg_flags_tab;
    const char * const **reg_names_tab;
    const mUINT32 ext_abi_idx=0;     /* ABI index for extension. (Forced to 0). */

    reg_flags_tab = TYPE_MEM_POOL_ALLOC_N(mUINT32*, Malloc_Mem_Pool,
					  (ISA_REGISTER_CLASS_MAX+1));
    memcpy(reg_flags_tab, ABI_PROPERTIES_base_props[abi_idx].reg_flags,
	   (ISA_REGISTER_CLASS_STATIC_MAX+1) * sizeof(mUINT32*));
    abi[abi_idx].reg_flags = reg_flags_tab;

    reg_names_tab = TYPE_MEM_POOL_ALLOC_N(const char* const *, Malloc_Mem_Pool,
					  (ISA_REGISTER_CLASS_MAX+1));
    memcpy(reg_names_tab, ABI_PROPERTIES_base_props[abi_idx].reg_names,
	   (ISA_REGISTER_CLASS_STATIC_MAX+1) * sizeof(const char * const*));
    abi[abi_idx].reg_names = reg_names_tab;
    
    for (ext=0; ext<ext_info.nb_ext; ext++) {
      int nb_rc = ext_info.ISA_tab[ext]->get_ISA_REGISTER_CLASS_tab_sz();
      if (nb_rc > 0) {
	const ABI_PROPERTIES *old_abi;
	old_abi = &ext_info.ISA_tab[ext]->get_ABI_PROPERTIES_tab()[ext_abi_idx];
	for (int rc = 0; rc < nb_rc; rc++) {
	  reg_flags_tab[ext_info.base_REGISTER_CLASS[ext]+rc] = old_abi->reg_flags[rc];
	  reg_names_tab[ext_info.base_REGISTER_CLASS[ext]+rc] = old_abi->reg_names[rc];
	}

	{ 
	  // Remap ABI properties attributes. 
	  // Core and Extension ABI properties identifiers are stored on mUINT32 as a mask (1<<id).
	  typedef ATTRIBUTE_MAP<mUINT32> CORE_ATTR_MAP;
	  typedef ATTRIBUTE_MAP<mUINT32> EXT_ATTR_MAP;
	  // The ABI properties mask for registers are stored both on mUINT32 in extension and core.
	  typedef ATTRIBUTE_REMAPPER<EXT_ATTR_MAP, CORE_ATTR_MAP, mUINT32, mUINT32 > EXT_ATTR_REMAPPER;
	  const NAME_VALUE<mUINT32> *core_attr_tab = (const NAME_VALUE<mUINT32> *)ABI_PROPERTIES_get_attribute_table();
	  const NAME_VALUE<mUINT32> *ext_attr_tab  = (const NAME_VALUE<mUINT32> *)ext_info.ISA_tab[ext]->get_ABI_PROPERTIES_attribute_tab();
	  INT    ext_attr_count = ext_info.ISA_tab[ext]->get_ABI_PROPERTIES_attribute_tab_sz();
	  CORE_ATTR_MAP core_attr_map(core_attr_tab, ABI_PROPERTIES_ATTRIBUTE_COUNT, CORE_ATTR_MAP::KIND_MASK);
	  EXT_ATTR_MAP ext_attr_map(ext_attr_tab, ext_attr_count, CORE_ATTR_MAP::KIND_MASK);
	  EXT_ATTR_REMAPPER remapper(ext_attr_map, core_attr_map);

	  for (int rc = 0; rc < nb_rc; rc++) {
	    mUINT32 *attr_walker = reg_flags_tab[ext_info.base_REGISTER_CLASS[ext]+rc];
	    for (int reg=0; reg<=ISA_REGISTER_MAX; reg++) {
	      if (!remapper.remapMask(attr_walker[reg], attr_walker[reg])) 
		RaiseErrorIncompatibleLibrary(ext_info.dll_tab[ext].dllname, 
					      "Incompatible target description,"
					      "ABI_PROPERTY attribute name in Extension not found in Core.");
	    }
	  }
	}
      }
    }
  }
  
  // Update targinfo pointers
  ABI_PROPERTIES_base_props = abi;
  ABI_PROPERTIES_Initialize();
}

/*
 * Return a pointer to the newly allocated API used to access to the 
 * ISA description of the specified extension.
 */
EXTENSION_ISA_Info *
Generate_EXTENSION_ISA_Info(const Extension_dll_t *dll_instance, BOOL verbose) {
  INT i;
  EXTENSION_ISA_Info  *isa_ext_access = NULL;
  const ISA_EXT_Interface_t *isa_ext;
  get_isa_extension_instance_t get_instance;
  get_instance = (get_isa_extension_instance_t)Get_dll_Symbol(dll_instance, "get_isa_extension_instance");
  isa_ext = (*get_instance)();

  // Check ISA extension compatibility and initialize the access API
  INT magic = isa_ext->magic;
  if (EXTENSION_Is_Supported_ISA_Revision(magic)) {
    if (verbose) {
      get_extension_name_t get_extname = (get_extension_name_t)Get_dll_Symbol(dll_instance, "get_extension_name");
      const char *extname = (*get_extname)();
      fprintf(TFile, "  [Extension '%s'] ISA API revision: lib=%d, compiler=%d\n", extname, magic, MAGIC_NUMBER_EXT_API);
    }
    isa_ext_access = new EXTENSION_ISA_Info(isa_ext);
  }
  else {
    char err_msg[256];
    sprintf(err_msg,
	    "Incompatible ISA API revision: lib=%d, compiler=%d.\n",
	    magic, MAGIC_NUMBER_EXT_ISA_API);
    RaiseErrorIncompatibleLibrary(dll_instance->dllname, err_msg);
  }

  return (isa_ext_access);
}

/*
 * Function called when an extension is incompatible with the core description
 */
void RaiseErrorIncompatibleLibrary(const char *name, const char *error_msg) {
  char err_msg[512];
  sprintf(err_msg,
	  "Failure.\n"
	  "### %s\n"
	  "### Please provide a compatible library.\n",
	  error_msg);
  ErrMsg(EC_Lib_Ext_Load, name, err_msg);
}

bool EXTENSION_Is_Extension_Present(const char* name) {
  int i;
  for (i=0; i<extension_count; i++) {
    if (!(strcmp(name, extension_tab[i].extname))) {
      return TRUE;
    }
  }
  return FALSE;
}

/*
 * Return the register class associated to the specified PREG number .
 */
ISA_REGISTER_CLASS EXTENSION_PREG_to_REGISTER_CLASS(PREG_NUM preg) {
  FmtAssert((extension_Preg_To_RegClass_table!=NULL),
			("Unexpected NULL extension_Preg_To_RegClass_table"));
  if (preg > Get_Static_Last_Dedicated_Preg_Offset() && preg  <= Get_Last_Dedicated_Preg_Offset_Func()) {
    return (extension_Preg_To_RegClass_table[preg - Get_Static_Last_Dedicated_Preg_Offset() - 1]);
  }
  FmtAssert((0),("Unexpected PREG out of extension bounds: %d", preg));
}

/*
 *TB map between gcc machine mode preg and open64 MTYPE
 */
TYPE_ID MachineMode_To_Mtype(machine_mode_t mode) {
  if (mode >= STATIC_COUNT_MACHINE_MODE) {
    return (Extension_MachineMode_To_Mtype_Array[mode - STATIC_COUNT_MACHINE_MODE]);
  }
  /* Modes other than those standard ones should never be seen, 
     the current callers to this function are:
     - the loader when creating new dynamic types.
     - the intrinsics arguments results mapping for extension/multi result
     intrinsincs. For the intrinsincs we restrict the types to the simple ones below.
     Note that unsigned types and bool type are not present, 
     which is not a problem for extension intrinscs but may be necessary
     for standard intrinsincs if the two flow are merged.
  */
  switch (mode) {
  case VOIDmode:
    return MTYPE_V;
  case QImode:
    return MTYPE_I1;
  case HImode:
    return MTYPE_I2;
  case SImode:
    return MTYPE_I4;
  case DImode:
    return MTYPE_I8;
  case SFmode:
    return MTYPE_F4;
  case DFmode:
    return MTYPE_F8;
  default:
    FmtAssert((0),("Unexpected mode to mtype conversion %d ", mode));
  }
  return MTYPE_UNKNOWN;
}

machine_mode_t Mtype_To_MachineMode(TYPE_ID mtype) {
  if (mtype > MTYPE_STATIC_LAST) {
    int i;
    for (i = 0;
	 i < Extension_MachineMode_To_Mtype_Array_Size;
	 i++) {
      if (Extension_MachineMode_To_Mtype_Array[i] == mtype) {
	return (STATIC_COUNT_MACHINE_MODE + i);
      }
    }
  }
  FmtAssert(0, ("Mtype_To_MachineMode() does not support standard mtype yet"));
  return 0;
}
				     
/*
 * Return the INTRINSIC that map a conversion from <src_ty> to <tgt_ty>.
 * At least one of the type is expected to be an extension one.
 */
INTRINSIC EXTENSION_Get_Convert_Intrinsic(TYPE_ID src_ty, TYPE_ID tgt_ty, BOOL ignore_sign) {
  INTRINSIC id = INTRINSIC_INVALID;
  if (equiv_type_enabled) {
    if (MTYPE_is_dynamic(src_ty)) {
      if (equiv_type_tab[src_ty].ctype == tgt_ty
	  || (ignore_sign
	      && MTYPE_complement(equiv_type_tab[src_ty].ctype) == tgt_ty)) {
	id = equiv_type_tab[src_ty].intrn_to_c;
      }
    }
    else if (MTYPE_is_dynamic(tgt_ty)) {
      if (equiv_type_tab[tgt_ty].ctype == src_ty
	  || (ignore_sign
	      && MTYPE_complement(equiv_type_tab[tgt_ty].ctype) == src_ty)) {
	id = equiv_type_tab[tgt_ty].intrn_to_ext;
      }
    }
  }
  return id;
}

/*
 * Return TRUE if conversion from src to tgt is allowed, FALSE otherwise.
 * At least one of the type is expected to be an extension one.
 */
BOOL EXTENSION_Are_Equivalent_Mtype(TYPE_ID src_ty, TYPE_ID tgt_ty) {
  BOOL equiv = FALSE;
  if (equiv_type_enabled) {
    BOOL ignore_sign = ! (Enable_Extension_Native_Support & EXTENSION_NATIVE_TYEQUIV_UNSIGNED_ONLY);
    equiv = (EXTENSION_Get_Convert_Intrinsic(src_ty, tgt_ty, ignore_sign) != INTRINSIC_INVALID);
  }
  return equiv;
}

TYPE_ID EXTENSION_Get_Equivalent_Mtype(TYPE_ID ext_ty) {
  TYPE_ID ty = MTYPE_UNKNOWN;
  if (equiv_type_enabled && MTYPE_is_dynamic(ext_ty)) {
    ty = equiv_type_tab[ext_ty].ctype;
  }
  return ty;
}


/** 
 * return clr intrinsic for mtype, if defined.
 * 
 * @param ty 
 * 
 * @return 
 */
INTRINSIC EXTENSION_Get_CLR_Intrinsic(TYPE_ID ty) {
  if (MTYPE_is_dynamic(ty)) {
    return equiv_type_tab[ty].clr_intrn;
  }
  return INTRINSIC_INVALID;
}


INT EXTENSION_Get_Equivalent_Mtype_Status() {
  return equiv_type_enabled;
}


void EXTENSION_Set_Equivalent_Mtype_Status(INT val) {
  equiv_type_enabled = val;
}


