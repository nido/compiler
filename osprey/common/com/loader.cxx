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



#include "../gccfe/extension_include.h"
#include "erglob.h"
#include "dyn_dll_api_access.h"
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
#include <stdlib.h>
#include <stdio.h>

#include "loader.h"
#include "lai_loader_api.h"

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

/*
 * Table containing all loaded extension
 */
static Extension_dll_t *extension_tab = (Extension_dll_t*)NULL;
static int extension_count = 0;


static void Add_Mtype(machine_mode_t mmode, const char *name, 
		      enum mode_class mclass, unsigned short mbitsize, 
		      unsigned char msize, unsigned char munitsize,
		      unsigned char mwidermode, machine_mode_t innermode, 
		      TYPE_ID mtype, unsigned short alignement) 
{
  /* Update open64 mtype*/
  if (mtype <= MTYPE_COUNT) {
    // MTYPE already exists, nothing to do
    return;
  }
  FmtAssert (mtype < MTYPE_MAX_LIMIT,
	     ("Two many MTYPES. Limit is %d", MTYPE_MAX_LIMIT));

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
TYPE_ID Add_MTypes(const Extension_dll_t *dll_instance, int **mtype_to_locrclass,  int *mtype_count, BOOL verbose)
{
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
	      modes[i].mwidermode, 
	      modes[i].innermode,
	      new_mtype,
	      modes[i].alignment);
    // Here, the new MTYPE has the id MTYPE_COUNT
    // Keep a mapping between this MTYPE and its local register file
    if (mtype_to_locrclass)
      mtype_to_local_rclass[i] = modes[i].local_REGISTER_CLASS_id;
  }
  if (mtype_to_locrclass)
    *mtype_to_locrclass = mtype_to_local_rclass;
  if (mtype_count)
    *mtype_count = count;
  return base_mtype + 1;
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
    intrn_info[i + INTRINSIC_COUNT + 1].is_by_val = btypes[i].is_by_val;
    intrn_info[i + INTRINSIC_COUNT + 1].is_pure = btypes[i].is_pure;
    intrn_info[i + INTRINSIC_COUNT + 1].has_no_side_effects = btypes[i].has_no_side_effects;
    intrn_info[i + INTRINSIC_COUNT + 1].never_returns = btypes[i].never_returns;
    intrn_info[i + INTRINSIC_COUNT + 1].is_actual = btypes[i].is_actual;
    intrn_info[i + INTRINSIC_COUNT + 1].is_cg_intrinsic = btypes[i].is_cg_intrinsic;
    intrn_info[i + INTRINSIC_COUNT + 1].c_name = (char*)btypes[i].c_name;
    intrn_info[i + INTRINSIC_COUNT + 1].specific_name = (char*)NULL;
    intrn_info[i + INTRINSIC_COUNT + 1].runtime_name =(char*)btypes[i].runtime_name;
    //TB: Complete proto info for this intrinsic
    Proto_Intrn_Info_Array[i + INTRINSIC_COUNT - INTRINSIC_STATIC_COUNT].argument_count = btypes[i].arg_count;

    // Add Mtype info: Convert machine mode proto info coming from the
    //dll into MTYPE. We need this info only for out and inout
    //parameters.
    //Out parameters
    TYPE_ID outmtype[10];
    for (int arg = 0; arg < btypes[i].arg_count; arg++) {
      TYPE_ID  mtype = MachineMode_To_Mtype(btypes[i].arg_type[arg]);
      outmtype[arg] = mtype;
    }

    INTRN_INOUT_TYPE inouttype[10];
    int nb_out = 0;
    int nb_in = 0;
    for (int arg = 0; arg < btypes[i].arg_count; arg++) {
      FmtAssert(btypes[i].arg_inout[arg] != BUILTARG_UNDEF,
		("Undef parameter type for intrinsic %s\n.", btypes[i].c_name));
      if (btypes[i].arg_inout[arg] == BUILTARG_OUT || btypes[i].arg_inout[arg] == BUILTARG_INOUT)
	nb_out++;
      if (btypes[i].arg_inout[arg] == BUILTARG_IN || btypes[i].arg_inout[arg] == BUILTARG_INOUT)
	nb_in++;
      // enum INTRN_INOUT_TYPE and enum BUILTARG_INOUT_TYPE are in the
      // same order, so just cast to mapp.
      inouttype[arg] = (INTRN_INOUT_TYPE)btypes[i].arg_inout[arg];
    }

    
    Proto_Intrn_Info_Array[i + INTRINSIC_COUNT - INTRINSIC_STATIC_COUNT].arg_type = 
      TYPE_MEM_POOL_ALLOC_N(TYPE_ID, Malloc_Mem_Pool, btypes[i].arg_count);
    memcpy(Proto_Intrn_Info_Array[i + INTRINSIC_COUNT - INTRINSIC_STATIC_COUNT].arg_type, 
	   outmtype, btypes[i].arg_count*sizeof(TYPE_ID));
    Proto_Intrn_Info_Array[i + INTRINSIC_COUNT - INTRINSIC_STATIC_COUNT].arg_inout = 
      TYPE_MEM_POOL_ALLOC_N(INTRN_INOUT_TYPE, Malloc_Mem_Pool, btypes[i].arg_count);
    memcpy(Proto_Intrn_Info_Array[i + INTRINSIC_COUNT - INTRINSIC_STATIC_COUNT].arg_inout, 
	   inouttype, btypes[i].arg_count*sizeof(TYPE_ID));

    Proto_Intrn_Info_Array[i + INTRINSIC_COUNT - INTRINSIC_STATIC_COUNT].argument_count = btypes[i].arg_count;
    Proto_Intrn_Info_Array[i + INTRINSIC_COUNT - INTRINSIC_STATIC_COUNT].arg_out_count = nb_out;
    Proto_Intrn_Info_Array[i + INTRINSIC_COUNT - INTRINSIC_STATIC_COUNT].arg_in_count = nb_in;

    if (btypes[i].return_type != VOIDmode && (nb_out != 0)) {
      char err_msg[256];
      sprintf(err_msg,
	      "Incompatible prototype for built-in %s. "
	      "A built-in cannot have out parameters and not returning void.",
	      btypes[i].c_name);
      RaiseErrorIncompatibleLibrary(dll_instance->dllname, err_msg);
    }

    //Initialize returned type
    Proto_Intrn_Info_Array[i + INTRINSIC_COUNT - INTRINSIC_STATIC_COUNT].return_type = MachineMode_To_Mtype(btypes[i].return_type);
    intrn_info[i + INTRINSIC_COUNT + 1].return_kind = INTRN_return_kind_for_mtype(MachineMode_To_Mtype(btypes[i].return_type));

    if (verbose) {
      fprintf(TFile, "    intrinsic[%d] = %s\n",
	      (i + INTRINSIC_COUNT + 1), (char*)btypes[i].runtime_name);
    }
  }
  INTRINSIC_COUNT += count;
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
		/*mwidermode*/ 0,/*innermode*/-1, 
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
  if (nb_mtype_to_add)
    //TB: Allocate mtype to mmode mapping for extension
    Extension_MachineMode_To_Mtype_Array = TYPE_MEM_POOL_ALLOC_N(TYPE_ID, Malloc_Mem_Pool, nb_mtype_to_add);
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
	      ("Symbol 'type_ext_interface' not found in extension dll '%s'\n",
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

  if (Extension_Is_Present) {
 

    BOOL verbose = Get_Trace(TP_EXTLOAD, 0xffffffff);
    
    // TODO: Load dlls...
    // ----------------
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
  }
  else {  // !Extension_Is_Present
    Init_Mtypes(0);
    Init_Intrinsics(0);
  }
}

/*
 * This function is responsible for settting PREG info To be called by
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
    const char ***reg_names_tab;
    const mUINT32 ext_abi_idx=0;     /* ABI index for extension */

    reg_flags_tab = TYPE_MEM_POOL_ALLOC_N(mUINT32*, Malloc_Mem_Pool,
					  (ISA_REGISTER_CLASS_MAX+1));
    memcpy(reg_flags_tab, ABI_PROPERTIES_base_props[abi_idx].reg_flags,
	   (ISA_REGISTER_CLASS_STATIC_MAX+1) * sizeof(mUINT32*));
    abi[abi_idx].reg_flags = reg_flags_tab;

    reg_names_tab = TYPE_MEM_POOL_ALLOC_N(const char**, Malloc_Mem_Pool,
					  (ISA_REGISTER_CLASS_MAX+1));
    memcpy(reg_names_tab, ABI_PROPERTIES_base_props[abi_idx].reg_flags,
	   (ISA_REGISTER_CLASS_STATIC_MAX+1) * sizeof(const char **));
    abi[abi_idx].reg_names = (const char *const**)reg_names_tab;
    
    for (ext=0; ext<ext_info.nb_ext; ext++) {
      int nb_rc = ext_info.ISA_tab[ext]->get_ISA_REGISTER_CLASS_tab_sz();
      if (nb_rc > 0) {
	const ABI_PROPERTIES *old_abi;
	old_abi = &ext_info.ISA_tab[ext]->get_ABI_PROPERTIES_tab()[ext_abi_idx];
	memcpy(&reg_flags_tab[ext_info.base_REGISTER_CLASS[ext]],
	       old_abi->reg_flags, nb_rc * sizeof(mUINT32*));
	memcpy(&reg_names_tab[ext_info.base_REGISTER_CLASS[ext]],
	       old_abi->reg_names, nb_rc * sizeof(const char**));

	{ // Remap ABI properties attributes if necessary
	  INT    nb_remap;
	  INT    ext_attr_count = ext_info.ISA_tab[ext]->get_ABI_PROPERTIES_attribute_tab_sz();
	  const ABI_PROPERTIES_ATTRIBUTE *core_attr_tab = ABI_PROPERTIES_get_attribute_table();
	  const ABI_PROPERTIES_ATTRIBUTE *ext_attr_tab  = ext_info.ISA_tab[ext]->get_ABI_PROPERTIES_attribute_tab();
	  COMPUTE_ATTR_REMAP_TABLE ( /* in_EXT_NAME        */ ext_info.dll_tab[ext].dllname,
				     /* in_COMPONENT_NAME  */ "ABI properties",
				     /* in_ATTR_TYPE       */ mUINT32,
				     /* in_ATTR_SUFFIX     */ UL,
				     /* in_ATTR_FORMAT     */ "%x",
				     /* in_CORE_ATTR_COUNT */ ABI_PROPERTIES_ATTRIBUTE_COUNT,
				     /* in_CORE_ATTR_TAB   */ core_attr_tab,
				     /* in_EXT_ATTR_COUNT  */ ext_attr_count,
				     /* in_EXT_ATTR_TAB    */ ext_attr_tab,
				     /* out_NB_REMAP       */ nb_remap
							      );
	  if (nb_remap>0) {
	    for (int rc = 0; rc < nb_rc; rc++) {
	      mUINT32 *attr_walker = &reg_flags_tab[ext_info.base_REGISTER_CLASS[ext]][rc];
	      for (int reg=0; reg<ISA_REGISTER_MAX; reg++) {
		REMAP_ATTR ( /* in_ATTR_TYPE   */ mUINT32,
			     /* in_ATTR_SUFFIX */ UL,
			     /* in_ATTR        */ *attr_walker,
			     /* out_ATTR       */ *attr_walker
						  );
		attr_walker++;
	      }
	    }
	  }
	  CLEANUP_ATTR_REMAP_TABLE();
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
      fprintf(TFile, "  ISA API revision: lib=%d, compiler=%d\n", magic, MAGIC_NUMBER_EXT_API);
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



/*
 * Return the register class associated to the specified PREG number .
 */
ISA_REGISTER_CLASS EXTENSION_PREG_to_REGISTER_CLASS(PREG_NUM preg) {
  FmtAssert((extension_Preg_To_RegClass_table!=NULL),
			("Unexpected NULL extension_Preg_To_RegClass_table"));
  if (preg > Get_Static_Last_Dedicated_Preg_Offset() && preg  <= Get_Last_Dedicated_Preg_Offset_Func()) {
    return (extension_Preg_To_RegClass_table[preg - Get_Static_Last_Dedicated_Preg_Offset() - 1]);
  }
  FmtAssert((0),("Unexpexted PREG out of extension bounds: %d", preg));
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
    FmtAssert((0),("Unexpexted mode to mtype conversion %d ", mode));
  }
  return MTYPE_UNKNOWN;
}
