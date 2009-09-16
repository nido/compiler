/* 

  Copyright (C) 2007 ST Microelectronics, Inc.  All Rights Reserved. 

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

/* This file provides extension specific code required by GCCFE, including:
 * - Addition of dynamic machine modes,
 * - Addition of dynamic builtins,
 * - Initialization of targinfo registers,
 * - Association between Pregs and targinfo registers
 */

#include "gnu_config.h"
#include "gnu/system.h"
#include "machmode.h"
#include "mtypes.h"
#include "builtins.h"
#include "wintrinsic.h"
extern "C" {
#include "tree.h"
}
#include "dyn_dll_api_access.h"
#include "dll_loader.h"
#include "loader.h"
#include "isa_loader_api.h"
#include "lai_loader_api.h"
#include "wfe_loader.h"
#include "tracing.h"
#include "errors.h"

#include "config.h" // For Extension_Is_Present
#include "tree.h"
#include "langhooks.h"
extern "C" {
#include "c-common.h"
}
#include "output.h"
#include "symtab_idx.h"
#include "targ_sim.h"

#include "gccfe_targinfo_interface.h" // ABI between targinfo and gccfe
#include "targ_isa_registers.h"       // For ISA_REGISTER_CLASS_INFO
#include "register_preg.h"

#ifdef TARGET_DUMP_INFO
static FILE *dumpinfofile;
#endif

// Mapping between gcc reg and open64 PREG
int *Map_Reg_To_Preg;
int Map_Reg_To_Preg_Size;

#define GCCFE_EXPORTED
// While the loader has not been initialized, set the counter to the
// max value.  This for the GCC initializaton of opttab_table, which
// occurs before the loader initialization.
machine_mode_t COUNT_MACHINE_MODE = MAX_LIMIT_MACHINE_MODE;

// Loader interface
static void Load_Machine_Type(const Extension_dll_t *dll_instance);
static void Load_Builtins(const Extension_dll_t *dll_instance);
static void Init_Builtins(int nb_builtins_to_add);
static void Init_Machmode(int nb_machmode_to_add);

// Extension tables
static INTRINSIC *INTRINSIC_For_Builtin = (INTRINSIC*)NULL;

// Tables to access the global MTYPE for a given extension rclass
static TYPE_ID *Rclass_to_Mtype;

// Keep access to extension info table
static Lai_Loader_Info_t *wfe_ext_info_table;

// Matching between open64 attributes and gcc attributes
static tree
get_gcc_attributes( extension_builtins_t btype) {
  return c_get_gcc_attributes(btype.never_returns, btype.is_pure && btype.has_no_side_effects, btype.has_no_side_effects);
}

/*
 * Add the dynamic builtins defined within specified extension dll
 */
static void WFE_Add_Builtins_dll(Extension_dll_t *dll_instance) {
  FmtAssert (dll_instance != NULL,
	     ("Extension instance not found"));
  const extension_builtins_t* btypes = dll_instance->hooks->get_builtins();
  unsigned int count = dll_instance->hooks->get_builtins_count();

  for (int i = 0; i < count; i++) {
    tree decl;
    if (wfe_ext_info_table->trace_on) {
      fprintf(TFile, "Adding %s gcc builtins:\n",
	      btypes[i].c_name);
    }
    if (strncmp (btypes[i].c_name, "__builtin_", strlen ("__builtin_")) != 0)
      FmtAssert (0,
		 ("Builtins with a non conform name"));
    tree function_type;
    tree return_type;

    if (btypes[i].return_type == VOIDmode) {
      if (wfe_ext_info_table->trace_on) {
	fprintf(TFile, "return void:%s [%d]\n",
		GET_MODE_NAME(btypes[i].return_type), btypes[i].return_type);
      }
      return_type = void_type_node;
    }
    else {
      if (wfe_ext_info_table->trace_on) {
	fprintf(TFile, "\treturn %s [%d]\n",
		GET_MODE_NAME(btypes[i].return_type),btypes[i].return_type);
      }
      return_type = (*lang_hooks.types.type_for_mode)
	(DLL_TO_GLOBAL_MMODE(dll_instance, btypes[i].return_type), 0);
    }

    int arg;
    tree arg_type_list = void_list_node;
    // Build Input parameter list
    for (arg = btypes[i].arg_count-1; arg >= 0; arg--) {
      if (wfe_ext_info_table->trace_on) {
	if (btypes[i].arg_inout[arg] == BUILTARG_OUT)
	  fprintf(TFile, "\tout");
	if (btypes[i].arg_inout[arg] == BUILTARG_INOUT)
	  fprintf(TFile, "\tinout");
	if (btypes[i].arg_inout[arg] == BUILTARG_IN)
	  fprintf(TFile, "\tin");
	fprintf(TFile, "\targ[%d]:%s [%d]\t", arg,
		GET_MODE_NAME(btypes[i].arg_type[arg]),btypes[i].arg_type[arg]);
      }

      // Build type for current parameter, with special handling for pointer type
      // (use of universal pointer type (void *))
      tree cur_arg_type;
      if (btypes[i].arg_type[arg] == PSImode) {
	cur_arg_type = build_pointer_type(void_type_node);
      }	else {
	cur_arg_type = (*lang_hooks.types.type_for_mode)(DLL_TO_GLOBAL_MMODE(dll_instance, btypes[i].arg_type[arg]), 0);
      }
      
      if (btypes[i].arg_inout[arg] == BUILTARG_IN) {
	arg_type_list = tree_cons (NULL_TREE,
				   cur_arg_type,
				   arg_type_list);
      } else {
	arg_type_list = tree_cons (NULL_TREE,
				   build_reference_type(cur_arg_type),
				   arg_type_list);
      }
    }

    if (wfe_ext_info_table->trace_on)
      fprintf(TFile, "\n");
    
    function_type = build_function_type (return_type,
					 arg_type_list);
    
    // Get GCC builtins attributes
    tree attrs = get_gcc_attributes(btypes[i]);
    decl = builtin_function_2 (btypes[i].c_name,
			       btypes[i].c_name + strlen ("__builtin_"),
			       function_type,
			       function_type,
			       DLL_TO_GLOBAL_BUILTIN(dll_instance, btypes[i].gcc_builtin_def),
			       BUILT_IN_NORMAL,
			       true,
			       true,
			       attrs);
    built_in_decls[(int) dll_instance->base_builtins + i] = decl;
  }
}

/*
 * Add the dynamic builtins defined by enabled extensions
 */
void WFE_Add_Builtins(void) {
  int i;
  if (!Extension_Is_Present) {
    return;
  }
  Extension_dll_t *extension_tab = Get_Extension_dll_tab( );

  FmtAssert (extension_tab != NULL,
	     ("Extension instance not found"));

  for (i=0; i<wfe_ext_info_table->nb_ext; i++) {
    if (wfe_ext_info_table->trace_on) {
      fprintf(TFile, "Extension '%s': Adding gcc builtins\n",
	      extension_tab[i].handler->dllname);
    }
    WFE_Add_Builtins_dll(&extension_tab[i]);
  }
}

INTRINSIC WFE_Intrinsic(enum built_in_function built)
{
  return INTRINSIC_For_Builtin[built - BUILT_IN_STATIC_COUNT];
}

/* ===========================================================================
 * This function initializes all gcc arrays for register properties,
 * These arrays are filled with targinfo data.
 ===========================================================================*/
void WFE_Loader_Initialize_Register () {
  int i, j;
  //Find the last gcc id of the last register of the core For that 2
  //solutions: find the max index in Additional_Core_Register_Names or the
  //size of Map_Reg_To_Preg / sizeof(PREG_NUM) -1
  int last_gcc_id = 0;
  int add_size = GCCTARG_Additional_Register_Names_Size();
  gcc_register_map_t *add_names = GCCTARG_Additional_Register_Names();

  char *call_used = GCCTARG_Initial_Call_Used_Regs();
  int reg_size = GCCTARG_Initial_Number_Of_Registers();

  char *fixed = GCCTARG_Initial_Fixed_Regs();

  int map_reg_size = GCCTARG_Map_Reg_To_Preg_Size();
  int *map_reg = GCCTARG_Map_Reg_To_Preg();

  for (i = 0; i < add_size; i++) {
    last_gcc_id = (last_gcc_id > add_names[i].number) ? last_gcc_id : add_names[i].number;
  }
  // Check that both solutions give the same result
  if (last_gcc_id != map_reg_size - 1)
    Fail_FmtAssertion("Map_Reg_To_Preg in config_target.cxx and Additional_Register_Names are not compatible");

  // Check that last_dedicated_preg_offset == ARRAY_SIZE(Map_Reg_To_Preg)
  if (Last_Dedicated_Preg_Offset != map_reg_size)
    Fail_FmtAssertion("Last_Dedicated_Preg_Offset and ARRAY_SIZE(Map_Reg_To_Preg) are not compatible");

  // Allocate memory for the new arrays
  // Register names
#if 0
  Additional_Register_Names = TYPE_MEM_POOL_ALLOC_N(gcc_register_map_t, Malloc_Mem_Pool,
						    add_size);
  memcpy(Additional_Register_Names, add_names , add_size * sizeof(gcc_register_map_t));
#endif
  Additional_Register_Names = add_names;
  Additional_Register_Names_Size = add_size;

  // Call used register array
#if 0
  Call_Used_Registers = TYPE_MEM_POOL_ALLOC_N(char, Malloc_Mem_Pool,
					      reg_size);
  memcpy(Call_Used_Registers, call_used , reg_size * sizeof(char));
#endif
  Call_Used_Registers = call_used;
  Number_Of_Registers = reg_size;

  // Fixed register array
#if 0
  Fixed_Registers = TYPE_MEM_POOL_ALLOC_N(char, Malloc_Mem_Pool,
					      Number_Of_Registers);
  memcpy(Fixed_Registers, fixed , reg_size * sizeof(char));
#endif
  Fixed_Registers = fixed;

  // register to open64 PREG mapping
#if 0
  Map_Reg_To_Preg = TYPE_MEM_POOL_ALLOC_N(PREG_NUM, Malloc_Mem_Pool,
					  map_reg_size);
  memcpy(Map_Reg_To_Preg, map_reg , map_reg_size * sizeof(PREG_NUM));
#endif
  Map_Reg_To_Preg = map_reg;
  Map_Reg_To_Preg_Size = map_reg_size;
}

/*
 * Initialize extension related info required by front-end:
 * - dynamic machine modes,
 * - dynamic builtins,
 */
void WFE_Init_Loader(void)
{
  if (Extension_Is_Present) {
    int i;
    int nb_ext_mtypes = 0;
    int nb_ext_intrinsics = 0;
    BOOL verbose = wfe_ext_info_table->trace_on;
    Extension_dll_t *extension_tab = Get_Extension_dll_tab( );
    FmtAssert (extension_tab != NULL,
	       ("Extension instance not found"));
    for (i=0; i<wfe_ext_info_table->nb_ext; i++) {
      nb_ext_mtypes     += extension_tab[i].hooks->get_modes_count();
      nb_ext_intrinsics += extension_tab[i].hooks->get_builtins_count();
    }

    // Initializations prior to effective additions
    Init_Machmode  (nb_ext_mtypes);     // GCC machine mode
    Init_Mtypes    (nb_ext_mtypes);     // Open64 MTYPE
    Init_Builtins  (nb_ext_intrinsics); // GCC builtins
    Init_Intrinsics(nb_ext_intrinsics); // Open64 intrinsics
  
    for (i=0; i<wfe_ext_info_table->nb_ext; i++) {
      int *Mtype_Local_RegisteClass;
      int Mtype_Count;
      // Add GCC machine mode
      Load_Machine_Type(&extension_tab[i]);

      // Add Open64 MTYPE
      // This function returns also a mapping between MTYPE and extension local register file
      TYPE_ID base_Mtype = Add_MTypes(&extension_tab[i], &Mtype_Local_RegisteClass, &Mtype_Count, verbose);

      // Build the Register class to mtype mappimg
      for (int type = Mtype_Count -1; type >= 0; type--) {
	ISA_REGISTER_CLASS rclass = Mtype_Local_RegisteClass[type]  + wfe_ext_info_table->base_REGISTER_CLASS[i];
	Rclass_to_Mtype[rclass - ISA_REGISTER_CLASS_STATIC_MAX - 1] = type + base_Mtype;
      }

      // Add GCC builtins
      Load_Builtins(&extension_tab[i]);

      // Add Open64 Intrinsics
      Add_Intrinsics(&extension_tab[i], verbose);
    }
    // Create new MTYPE for multiple result intrinsics
    Add_Composed_Mtype();
  }
  else {
    // No extension
    Init_Machmode  (0); // GCC machine mode
    Init_Mtypes    (0); // Open64 MTYPE
    Init_Builtins  (0); // GCC builtins
    Init_Intrinsics(0); // Open64 intrinsics
  }

  //Dump info
#ifdef TARGET_DUMP_INFO
  if (TARGET_DUMP_INFO) {
    dumpinfofile = fopen("info.txt", "w");
    fprintf(dumpinfofile,"Available Machine Mode\n");
    for (int j = 0; j < NUM_MACHINE_MODES; j++)
      fprintf(dumpinfofile, "\t%s\t%d\t%s\n",GET_MODE_NAME (j), GET_MODE_BITSIZE(j),VECTOR_MODE_P(j) ?"vector":"scalar");
    
    fprintf(dumpinfofile,"Available Builtins\n");
    for (int j = 0; j < BUILT_IN_COUNT; j++)
      fprintf(dumpinfofile, "\t%s\n", built_in_names[j]);

    //Print Additional_Register_Names and Map_Reg_To_Preg
    fprintf(dumpinfofile,"gcc_id\tname\tfixed\tcall_used\n");
    for (int i =0; i < Additional_Register_Names_Size; i++)
      fprintf(dumpinfofile,"[%d]\t%s\t%d\t%d\n",Additional_Register_Names[i].number, Additional_Register_Names[i].name, Fixed_Registers[i], Call_Used_Registers[i]);
  }
#endif
}
tree dynamic_tree_type[MAX_LIMIT_MACHINE_MODE - STATIC_COUNT_MACHINE_MODE];
tree dynamic_tree_unsigned_type[MAX_LIMIT_MACHINE_MODE - STATIC_COUNT_MACHINE_MODE];

/*
 * Add the specified dynamic machine mode in FE structure
 */
static void Add_New_Machine_Type(machine_mode_t mmode, const char *name, 
				 enum mode_class mclass, unsigned short mbitsize, 
				 unsigned short msize, unsigned short munitsize,
				 unsigned char mwidermode, machine_mode_t innermode, 
				 TYPE_ID mtype) 
{
  if (wfe_ext_info_table->trace_on) {
    fprintf(TFile,"Adding %s gcc machine mode:\n", name);
    fprintf(TFile,"\tnumber:%d\n", COUNT_MACHINE_MODE);
    fprintf(TFile,"\tsize:%d\n", msize);
    fprintf(TFile,"\tinnermode:%s [%d]\n", GET_MODE_NAME(innermode),innermode);
  }
  FmtAssert(COUNT_MACHINE_MODE < MAX_LIMIT_MACHINE_MODE,
	    ("Limit of machine mode reached."));
  mode_name[COUNT_MACHINE_MODE] = name;
  mode_class[COUNT_MACHINE_MODE] = mclass;
  mode_bitsize[COUNT_MACHINE_MODE] = mbitsize;
  mode_size[COUNT_MACHINE_MODE] = msize;
  mode_unit_size[COUNT_MACHINE_MODE] = munitsize;
  mode_wider_mode[COUNT_MACHINE_MODE] = mwidermode;
  mode_mask_array[COUNT_MACHINE_MODE] =  ((mbitsize) >= HOST_BITS_PER_WIDE_INT) ? ~(unsigned HOST_WIDE_INT) 0 : ((unsigned HOST_WIDE_INT) 1 << (mbitsize)) - 1;
  inner_mode_array[COUNT_MACHINE_MODE] = innermode;
  /* Tree gcc tree type */
  /* call (*lang_hooks.types.type_for_mode) to get the tree associated
     to the subtype */
  dynamic_tree_type[COUNT_MACHINE_MODE - STATIC_COUNT_MACHINE_MODE] = make_vector (COUNT_MACHINE_MODE, (*lang_hooks.types.type_for_mode)(innermode, 0), 0);
  dynamic_tree_unsigned_type[COUNT_MACHINE_MODE - STATIC_COUNT_MACHINE_MODE] = make_vector (COUNT_MACHINE_MODE, (*lang_hooks.types.type_for_mode)(innermode, 1), 1);

  COUNT_MACHINE_MODE = (machine_mode_t)((int)COUNT_MACHINE_MODE + 1);
}

/*
 * Machine mode initialization
 */
static void Init_Machmode(int nb_machmode_to_add)
{
  COUNT_MACHINE_MODE = STATIC_COUNT_MACHINE_MODE;
}

/*
 * Builtins table initialization:
 *   This function is responsible for heap array allocation and copy
 *   static value to heap arrays
 */
static void Init_Builtins(int nb_builtins_to_add)
{
  BUILT_IN_COUNT = BUILT_IN_STATIC_COUNT;
  built_in_decls = TYPE_MEM_POOL_ALLOC_N(tree,
					 Malloc_Mem_Pool,
					 (BUILT_IN_STATIC_COUNT + nb_builtins_to_add));

  if (nb_builtins_to_add == 0) {
    // Nothing to allocate, juste reference static tables
    built_in_names = (const char**)built_in_names_static;
  }
  else {
    int i;
    built_in_names = TYPE_MEM_POOL_ALLOC_N(const char *,
					  Malloc_Mem_Pool,
					  (BUILT_IN_STATIC_COUNT + nb_builtins_to_add));
    memcpy(built_in_names,
	   built_in_names_static,
	   BUILT_IN_STATIC_COUNT * sizeof(const char *));

    INTRINSIC_For_Builtin = TYPE_MEM_POOL_ALLOC_N(INTRINSIC,
						  Malloc_Mem_Pool,
						  nb_builtins_to_add);

  }
}
tree *dynamic_builtin_types;
unsigned int DYNAMIC_BUILTIN_TYPES_COUNT;

static void Add_New_Builtins(const Extension_dll_t *dll_instance,
			     extension_builtins_t butype) 
{
  built_in_names[BUILT_IN_COUNT] = butype.c_name;
  INTRINSIC_For_Builtin[BUILT_IN_COUNT - BUILT_IN_STATIC_COUNT] = DLL_TO_GLOBAL_INTRINSIC(dll_instance, butype.open64_intrincic);

  BUILT_IN_COUNT = (enum built_in_function)((int)BUILT_IN_COUNT + 1);
}

/* ========================================================= */

/* Loader specific functions: function to put in the loader  */

/* ========================================================= */

static void Load_Machine_Type(const Extension_dll_t *dll_instance)
{
  int i;
  unsigned int count = dll_instance->hooks->get_modes_count();
  machine_mode_t base = dll_instance->hooks->get_modes_base_count();
  const extension_machine_types_t* modes = dll_instance->hooks->get_modes();
  if (wfe_ext_info_table->trace_on)
    fprintf(TFile,"Adding machine mode from %d (last static mmode is %s)\n", STATIC_COUNT_MACHINE_MODE, GET_MODE_NAME(STATIC_COUNT_MACHINE_MODE-1));
  for (i = 0; i < count; i++) {
    Add_New_Machine_Type(DLL_TO_GLOBAL_MMODE(dll_instance, modes[i].mmode),
			 modes[i].name, 
			 modes[i].mclass, 
			 modes[i].mbitsize,
			 modes[i].msize,
			 modes[i].munitsize,
			 modes[i].mwidermode, 
			 modes[i].innermode,
			 DLL_TO_GLOBAL_MTYPE(dll_instance, modes[i].mtype));
  }
}

static void Load_Builtins(const Extension_dll_t *dll_instance)
{
  int i, count = dll_instance->hooks->get_builtins_count();
  const extension_builtins_t* btypes = dll_instance->hooks->get_builtins();
  for (i = 0; i < count; i++) {
    Add_New_Builtins(dll_instance, btypes[i]);
  }
}

/*
 * Return the register class associated to the specified PREG number .
 */
static TYPE_ID  EXTENSION_REGISTER_CLASS_to_MTYPE(ISA_REGISTER_CLASS rclass) {
  FmtAssert((Rclass_to_Mtype!=NULL),
			("Unexpected NULL Rclass_to_Mtype"));
  if (rclass > ISA_REGISTER_CLASS_STATIC_MAX && rclass  <= ISA_REGISTER_CLASS_MAX) {
    return (Rclass_to_Mtype[rclass - ISA_REGISTER_CLASS_STATIC_MAX - 1]);
  }
  FmtAssert((0),("Unexpected Register class out of extension bounds: %d", rclass));
}

/*
 * Return MTYPE associated to specified extension PREG
 */
TYPE_ID EXTENSION_Get_Mtype_For_Preg(PREG_NUM preg)
{
  // First check that this PREG is an extension PREG
  INT static_last_dedicated_preg_offset = Get_Static_Last_Dedicated_Preg_Offset();
  FmtAssert ((preg > static_last_dedicated_preg_offset) && (preg <=  Last_Dedicated_Preg_Offset),
	     ("EXTENSION_Get_Mtype_For_Preg: invalid preg number %d", preg));

  // Get the register class associated to this PREG
  ISA_REGISTER_CLASS rclass = EXTENSION_PREG_to_REGISTER_CLASS(preg);
  // Get the MTYPE associated to this Register Class
  TYPE_ID type = EXTENSION_REGISTER_CLASS_to_MTYPE(rclass);
  
  return type;
}

/* ===========================================================================
 * This function initializes all gcc specific information for extension
 * description.
 * This mainly covers the targinfo description for register class,
 * ABI, ...
 * ===========================================================================*/
static void Gccfe_Initialize_Extension_Loader () {
  int i, j;
  if (!Extension_Is_Present) {
    return;
  }
  Extension_dll_t *extension_tab;
  BOOL verbose = 0;
#ifdef Is_True_On
  verbose = (getenv("GCCEXT_DEBUG") != NULL);
  if (verbose) {
    fprintf(stderr,"verbose: %d, %s\n",verbose, getenv("GCCEXT_DEBUG"));
    if (Get_Trace_File() == stdout)
      Set_Trace_File("gccext_debug.t");
  }
#endif
  // Allocate and initialize wfe specific extension table
  wfe_ext_info_table = TYPE_MEM_POOL_ALLOC_N(Lai_Loader_Info_t, Malloc_Mem_Pool, 1);
  // Load extension dlls and count extension specific mtypes and intrinsics
  Load_Extension_dlls(verbose);
  extension_tab = Get_Extension_dll_tab( );
  wfe_ext_info_table->nb_ext = Get_Extension_dll_count( );
  wfe_ext_info_table->trace_on  = verbose;
  int extension_count = wfe_ext_info_table->nb_ext;
  const EXTENSION_HighLevel_Info **ext_tab;
  ext_tab = TYPE_MEM_POOL_ALLOC_N(const EXTENSION_HighLevel_Info*, Malloc_Mem_Pool, extension_count);
  for (i=0; i<extension_count; i++) {
    ext_tab[i] = extension_tab[i].hooks;
  }

  if (wfe_ext_info_table->trace_on) {
    fprintf(TFile,"%s Running Targinfo Extension Loader\n%s", DBar, DBar);
  }

  // Base offset for each element of each extension
  wfe_ext_info_table->ISA_tab                = TYPE_MEM_POOL_ALLOC_N(EXTENSION_ISA_Info*, Malloc_Mem_Pool, extension_count);
  wfe_ext_info_table->base_REGISTER_CLASS    = TYPE_MEM_POOL_ALLOC_N(int, Malloc_Mem_Pool, extension_count);
  wfe_ext_info_table->base_PREG              = TYPE_MEM_POOL_ALLOC_N(int, Malloc_Mem_Pool, extension_count);
  
  // Compute cumulated bounds
  // ------------------------
  int rc_max    = ISA_REGISTER_CLASS_STATIC_MAX;
  int preg_max = Get_Static_Last_Dedicated_Preg_Offset();
  // TODO: to be shared with lai_loader instance
  for (i=0; i < extension_count; i++) {

    // Get ISA Extension description accessor
    wfe_ext_info_table->ISA_tab[i] = Generate_EXTENSION_ISA_Info(extension_tab[i].handler, wfe_ext_info_table->trace_on);

    wfe_ext_info_table->base_REGISTER_CLASS[i] = rc_max+1;
    int rc_in_ext    = wfe_ext_info_table->ISA_tab[i]->get_ISA_REGISTER_CLASS_tab_sz();
    const ISA_REGISTER_CLASS_INFO *rc_tab = wfe_ext_info_table->ISA_tab[i]->get_ISA_REGISTER_CLASS_tab();
    rc_max  += rc_in_ext;
    
    //TB: Compute nb of registers to add to the core
    wfe_ext_info_table->base_PREG[i] = preg_max+1;
    for (int regcl = 0; 
	 regcl < rc_in_ext; 
	 regcl++) {
      int first_isa_reg  = rc_tab[regcl].min_regnum;
      int last_isa_reg   = rc_tab[regcl].max_regnum;
      int register_count = last_isa_reg - first_isa_reg + 1;
      preg_max += register_count;
    }
  }

  FmtAssert((rc_max <= ISA_REGISTER_CLASS_MAX_LIMIT),
			("Too many register classes defined by extensions"));

  // Update targinfo maximum bounds
  ISA_REGISTER_CLASS_MAX    = rc_max;

  //Reinitialize register class to preg offset
  if (preg_max != Get_Static_Last_Dedicated_Preg_Offset()) {
    Set_Last_Dedicated_Preg_Offset(preg_max);
    Reset_RegisterClass_To_Preg();
  }

  //Initialize the local array to map register class to mtype for
  //extension rclass
  Rclass_to_Mtype = TYPE_MEM_POOL_ALLOC_N(TYPE_ID, Malloc_Mem_Pool,
					  ISA_REGISTER_CLASS_MAX - ISA_REGISTER_CLASS_STATIC_MAX);

  // ==========================================================================
  //
  // Initialize the various targinfo part. Note that the order is relevant!!
  //
  // ==========================================================================

  Initialize_ISA_RegisterClasses(*wfe_ext_info_table);
  Initialize_ABI_Properties(*wfe_ext_info_table);
}


//GCC targinfo initialization
void TI_Initialize_Extension_Loader(void) {
  if (Extension_Is_Present) {
    Gccfe_Initialize_Extension_Loader ();
  }
}

