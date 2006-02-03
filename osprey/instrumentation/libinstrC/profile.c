//
// Description:
//
// During instrumentation, calls to the following procedures are
// inserted into the WHIRL code.  When invoked, these procedures
// initialize, perform, and finalize frequency counts.
//
// ====================================================================
// ====================================================================


#include "myalloc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "profile_interface.h"
#include "profile_aux.h"
#include "libfb_info.h"
#include "dump.h"
// ====================================================================


static char *output_filename = NULL;
static BOOL unique_output_filename = FALSE; 
// ====================================================================
// Hash Table that can be accessed with the PC address of a PU
// and returns a pointer to the Profile handle for that PU.
HASH_MAP PU_Profile_Handle_Table = NULL;
static PROFILE_PHASE instrumentation_phase_num = PROFILE_PHASE_NONE;
static void Incr_Branch_Taken(PU_PROFILE_HANDLE pu_handle, INT32 id);
static void Incr_Branch_Not_Taken(PU_PROFILE_HANDLE pu_handle, INT32 id);
static void Incr_Right_Taken(PU_PROFILE_HANDLE pu_handle, INT32 id);
static void Incr_Neither_Taken(PU_PROFILE_HANDLE pu_handle, INT32 id);
static void Set_Instrumentation_Phase_Num(PROFILE_PHASE phase_num);
extern PROFILE_PHASE Instrumentation_Phase_Num(void);


#define NOT_STARTED -1
#define TO_BE_STARTED 0
#define START_IN_PROGRESS 1
#define STARTED 2
#define FINISHED_IN_PROGRESS 3
#define FINISHED 4 
#define OFF 5
static BOOL IN_LIBINSTR = FALSE;   
/* IN_LIBINSTR : to prevent instrumenting code from libinstr */
static int LIB_STATE = TO_BE_STARTED;
static int SAVE_LIB_STATE = TO_BE_STARTED;
/* Define standard functions: */
#ifdef MAX
# undef MAX
#endif
#ifdef MIN
# undef MIN
#endif
#define MAX(a,b)	((a>=b)?a:b)
#define MIN(a,b)	((a<=b)?a:b)

inline INT32 Max(INT32 i, INT32 j)
{
  return MAX(i,j);
}
inline INT32 Min(INT32 i, INT32 j)
{
  return MIN(i,j);
}



// ====================================================================
// ====================================================================

// Given the PC address of a PU, index into a hash table and 
// retrieve the profile handle for the PU. If the handle is
// NULL, create it.

static PU_PROFILE_HANDLE PU_Profile_Handle_new(char *fname, char *pname, INT32 c_sum) {
  PU_PROFILE_HANDLE pu_profile_handle;
  pu_profile_handle = (PU_PROFILE_HANDLE) MYMALLOC(sizeof(struct PU_Profile_Handle));
  (pu_profile_handle)->checksum = c_sum;
  (pu_profile_handle)->file_name = (char *)MYMALLOC(strlen(fname) + 1);
  (pu_profile_handle)->pu_name = (char *)MYMALLOC(strlen(pname) + strlen("/") + strlen(fname) + 1);
  strcpy((pu_profile_handle)->file_name, fname);
  strcpy((pu_profile_handle)->pu_name, fname);
  strcat((pu_profile_handle)->pu_name,"/");
  strcat((pu_profile_handle)->pu_name,pname);

  (pu_profile_handle)->LIBFB_Info_Invoke_Table = NULL;
  (pu_profile_handle)->LIBFB_Info_Branch_Table = NULL;
  (pu_profile_handle)->Switch_Profile_Table = NULL;
  (pu_profile_handle)->Compgoto_Profile_Table = NULL;
  (pu_profile_handle)->LIBFB_Info_Loop_Table = NULL;
  (pu_profile_handle)->LIBFB_Info_Circuit_Table = NULL;
  (pu_profile_handle)->LIBFB_Info_Call_Table = NULL;
  return pu_profile_handle;
}

static PU_PROFILE_HANDLE PU_Profile_Handle_del(PU_PROFILE_HANDLE pu_profile_handle) {
  MYFREE(pu_profile_handle->file_name);
  MYFREE(pu_profile_handle->pu_name);
  MYFREE(pu_profile_handle->LIBFB_Info_Invoke_Table->data);
  MYFREE(pu_profile_handle->LIBFB_Info_Invoke_Table);
  pu_profile_handle->LIBFB_Info_Invoke_Table = NULL;

  if (pu_profile_handle->LIBFB_Info_Branch_Table) {
    MYFREE(pu_profile_handle->LIBFB_Info_Branch_Table->data);
    MYFREE(pu_profile_handle->LIBFB_Info_Branch_Table);
    pu_profile_handle->LIBFB_Info_Branch_Table = NULL;
  }

  if (pu_profile_handle->Switch_Profile_Table)
  {
    size_t size, i;
    size = pu_profile_handle->Switch_Profile_Table->size;
    for (i = 0; i < size; i++) {
      Switch_Profile data = pu_profile_handle->Switch_Profile_Table->data[i];
      MYFREE(data.targets_profile);
      data.targets_profile = NULL;
      MYFREE(data.targets_case_value);
      data.targets_case_value = NULL;
    }
  }

  if (pu_profile_handle->Switch_Profile_Table) {
    MYFREE(pu_profile_handle->Switch_Profile_Table->data);
    MYFREE(pu_profile_handle->Switch_Profile_Table);
    pu_profile_handle->Switch_Profile_Table = NULL; 
  }

  if (pu_profile_handle->Compgoto_Profile_Table)
  {
    size_t size, i;
    size = pu_profile_handle->Compgoto_Profile_Table->size;
    for (i = 0; i < size; i++) {
      Compgoto_Profile data = pu_profile_handle->Compgoto_Profile_Table->data[i];
      MYFREE(data.targets_profile);
      data.targets_profile = NULL;
    }
  }
  if (pu_profile_handle->Compgoto_Profile_Table) {
    MYFREE(pu_profile_handle->Compgoto_Profile_Table->data);
    MYFREE(pu_profile_handle->Compgoto_Profile_Table);
    pu_profile_handle->Compgoto_Profile_Table = NULL; 
  }

  if (pu_profile_handle->LIBFB_Info_Loop_Table) {
    MYFREE(pu_profile_handle->LIBFB_Info_Loop_Table->data);
    MYFREE(pu_profile_handle->LIBFB_Info_Loop_Table);
    pu_profile_handle->LIBFB_Info_Loop_Table = NULL; 
  }

  if (pu_profile_handle->LIBFB_Info_Circuit_Table) {
    MYFREE(pu_profile_handle->LIBFB_Info_Circuit_Table->data);
    MYFREE(pu_profile_handle->LIBFB_Info_Circuit_Table);
    pu_profile_handle->LIBFB_Info_Circuit_Table = NULL; 
  }

  if (pu_profile_handle->LIBFB_Info_Call_Table) {
    MYFREE(pu_profile_handle->LIBFB_Info_Call_Table->data);
    MYFREE(pu_profile_handle->LIBFB_Info_Call_Table);
    pu_profile_handle->LIBFB_Info_Call_Table = NULL; 
  }
  
  MYFREE (pu_profile_handle);
  return NULL;
}

// ====================================================================

// ====================================================================

// For each PU, we want to a one-time initialization of the
// tables that maintain profile information of invoke nodes.
// Given a pu_handle and the number of invoke nodes in that PU, we
// initialize the Call_Profile_Table in the pu handle with the appropriate
// number of entries. This routine may be invoked multiple times from
// a PU, but the initialization is done only the first time.

void 
__profile_invoke_init(void *pu_handle, INT32 num_invokes)
{
  LIBFB_Info_Invoke_Vector *Inv_Table = NULL;

  if (LIB_STATE != STARTED) return;
  if (IN_LIBINSTR == TRUE) return;
  IN_LIBINSTR = TRUE;

  Inv_Table = ((PU_PROFILE_HANDLE)pu_handle)->LIBFB_Info_Invoke_Table;
  if (Inv_Table == NULL) {
    Inv_Table = (LIBFB_Info_Invoke_Vector *)MYMALLOC(sizeof(LIBFB_Info_Invoke_Vector));
    Inv_Table->size = num_invokes;
    Inv_Table->data = (LIBFB_Info_Invoke*)MYMALLOC(num_invokes * sizeof(LIBFB_Info_Invoke));
    memset(Inv_Table->data, 0, num_invokes * sizeof(LIBFB_Info_Invoke));
    ((PU_PROFILE_HANDLE)pu_handle)->LIBFB_Info_Invoke_Table = Inv_Table;
  }
  IN_LIBINSTR = FALSE;
}

// Update entry count for a invoke
void 
__profile_invoke(void *pu_handle, INT32 invoke_id)
{
  LIBFB_Info_Invoke_Vector *Inv_Table = NULL;

  if (LIB_STATE != STARTED) return;
  if (IN_LIBINSTR == TRUE) return;
  IN_LIBINSTR = TRUE;

  Inv_Table = ((PU_PROFILE_HANDLE)pu_handle)->LIBFB_Info_Invoke_Table;
  (Inv_Table->data[invoke_id].freq_invoke)++;
  IN_LIBINSTR = FALSE;
}

// ====================================================================

// For each PU, we want to a one-time initialization of the 
// tables that maintain profile information of branch nodes. 
// Given a pu_handle and the number of conditional branches
// in that PU, we initialize the Br_Table in the pu handle
// with the appropriate number of entries. 
// This routine may be invoked multiple times from a PU, but the
// initialization is done only the first time.

void
__profile_branch_init(void *pu_handle, INT32 num_branches)
{
  LIBFB_Info_Branch_Vector *Br_Table = NULL;

  if (LIB_STATE != STARTED) return;
  if (IN_LIBINSTR == TRUE) return;
  IN_LIBINSTR = TRUE;

  Br_Table = ((PU_PROFILE_HANDLE)pu_handle)->LIBFB_Info_Branch_Table;
  if (Br_Table == NULL) {
    Br_Table = (LIBFB_Info_Branch_Vector *)MYMALLOC(sizeof(LIBFB_Info_Branch_Vector));
    Br_Table->size = num_branches;
    Br_Table->data = (LIBFB_Info_Branch*)MYMALLOC(num_branches * sizeof(LIBFB_Info_Branch));
    memset(Br_Table->data, 0, num_branches * sizeof(LIBFB_Info_Branch));
    ((PU_PROFILE_HANDLE)pu_handle)->LIBFB_Info_Branch_Table = Br_Table;
  }
  IN_LIBINSTR = FALSE;
}

// Update appropriate profile information for a branch.
void
__profile_branch(void *pu_handle, INT32 branch_id, BOOL taken)
{
  if (LIB_STATE != STARTED) return;
  if (IN_LIBINSTR == TRUE) return;
  IN_LIBINSTR = TRUE;

  if (taken)
       Incr_Branch_Taken(((PU_PROFILE_HANDLE)pu_handle), branch_id);
  else 
       Incr_Branch_Not_Taken(((PU_PROFILE_HANDLE)pu_handle), branch_id);

  IN_LIBINSTR = FALSE;
}

// Given a PU handle, retrieve the branch table and increment
// taken count for id.

static void Incr_Branch_Taken(PU_PROFILE_HANDLE pu_handle, INT32 id) 
{
  LIBFB_Info_Branch_Vector *Br_Table = NULL;

  Br_Table = pu_handle->LIBFB_Info_Branch_Table;
  Br_Table->data[id].freq_taken++;
} 

// Given a PU handle, retrieve the branch table and increment
// not-taken count for id.

static void Incr_Branch_Not_Taken(PU_PROFILE_HANDLE pu_handle, INT32 id) 
{
  LIBFB_Info_Branch_Vector *Br_Table = NULL;
  Br_Table = pu_handle->LIBFB_Info_Branch_Table;
  Br_Table->data[id].freq_not_taken++;
}

// ====================================================================

// For each PU, we want a one-time initialization of the 
// tables that maintain profile information of Switch nodes. 
// Given a pu_handle, the number of Switches in that PU and an
// array that reprents the number of targets for each Switch in the PU,
// we initialize the Switch_Table in the pu handle with the appropriate 
// number of entries. This routine may be invoked multiple times from a 
// PU, but the initialization is done only the first time.

// switch_num_targets[i] gives the number of targets for the ith Switch
// in the PU.

void
__profile_switch_init(void *pu_handle,
		    INT32 num_switches, INT32 *switch_num_targets,
		    INT32 num_case_values, INT64 *case_values)
{
  Switch_Profile_Vector *Switch_Table = NULL;

  if (LIB_STATE != STARTED) return;
  if (IN_LIBINSTR == TRUE) return;
  IN_LIBINSTR = TRUE;

  Switch_Table = ((PU_PROFILE_HANDLE)pu_handle)->Switch_Profile_Table;
  if (Switch_Table == NULL) {
    int i;
    Switch_Table = (Switch_Profile_Vector *)MYMALLOC(sizeof(Switch_Profile_Vector));
    Switch_Table->size = num_switches;
    Switch_Table->data = (Switch_Profile*)MYMALLOC(num_switches * sizeof(Switch_Profile));

    for( i = 0; i < num_switches; i++) {
      int j;
      INT32 num_targets = *switch_num_targets;
      
      Switch_Table->data[i].targets_profile = (INT64*)MYMALLOC((num_targets + 1) * sizeof(INT64));
      memset(Switch_Table->data[i].targets_profile, 0, (num_targets + 1) * sizeof(INT64));
      Switch_Table->data[i].targets_profile_size = num_targets + 1;
      Switch_Table->data[i].targets_case_value = (INT64*)MYMALLOC(num_targets  * sizeof(INT64));
      
      for (j = 0; j < num_targets; j++) {
	Switch_Table->data[i].targets_case_value[j] = case_values[j];
      }
      case_values += num_targets;
      ++switch_num_targets;
    }
    // Should now be true:  case_index == num_case_values
    ((PU_PROFILE_HANDLE)pu_handle)->Switch_Profile_Table = Switch_Table;
  }
  IN_LIBINSTR = FALSE;
}

// Update appropriate profile information for an Switch
// Given a PU handle, retrieve the Switch table; use this table 
// and the switch_id to retrieve the vector representing the
// possible targets for this Switch; target takes a 
// value between 0 and n-1 where n is the number of targets
// for the Switch. Increment the profile information for 
// the appropriate target represented by 'target'.

void
__profile_switch(void *pu_handle, INT32 switch_id, INT32 case_value, 
	       INT32 num_targets)
{
  INT32 t, target = -1;  // default branch
  Switch_Profile_Vector *switch_table = NULL;
  INT64 *targets_case_value = NULL;

  if (LIB_STATE != STARTED) return;
  if (IN_LIBINSTR == TRUE) return;
  IN_LIBINSTR = TRUE;

  switch_table = ((PU_PROFILE_HANDLE)pu_handle)->Switch_Profile_Table;

  targets_case_value = switch_table->data[switch_id].targets_case_value;

  // Which branch corresponds to case_value?
  for (t = 0; t < num_targets; t++) {
    INT64 tcv = targets_case_value[t];
    if ( tcv == case_value) {
      target = t;
    }
  }
  switch_table->data[switch_id].targets_profile[target + 1]++; // 0 is default
  IN_LIBINSTR = FALSE;
}

// ====================================================================
// For each PU, we want to a one-time initialization of the 
// tables that maintain profile information of Compgoto nodes. 
// Given a pu_handle, the number of Compgotos in that PU and an
// array that reprents the number of targets for each Compgoto in the PU,
// we initialize the Compgoto_Table in the pu handle with the appropriate 
// number of entries. This routine may be invoked multiple times from a 
// PU, but the initialization is done only the first time.
// compgoto_num_targets[i] gives the number of targets for the ith Compgoto
// in the PU.
void
__profile_compgoto_init(void *pu_handle, INT32 num_compgotos,
		      INT32 *compgoto_num_targets)
{
  Compgoto_Profile_Vector *Compgoto_Table = NULL;

  if (LIB_STATE != STARTED) return;
  if (IN_LIBINSTR == TRUE) return;
  IN_LIBINSTR = TRUE;

  Compgoto_Table = ((PU_PROFILE_HANDLE)pu_handle)->Compgoto_Profile_Table;

  if (Compgoto_Table == NULL) {
    INT32 i;
    Compgoto_Table = (Compgoto_Profile_Vector *)MYMALLOC(sizeof(Compgoto_Profile_Vector));
    Compgoto_Table->size = num_compgotos;
    Compgoto_Table->data = (Compgoto_Profile*)MYMALLOC(num_compgotos * sizeof(Compgoto_Profile));

     for (i = 0; i < num_compgotos; i++) {
	 INT32 num_targets = *compgoto_num_targets + 1;

	 Compgoto_Table->data[i].targets_profile = (INT64*)MYMALLOC((num_targets + 1) * sizeof(INT64));
	 Compgoto_Table->data[i].targets_profile_size = num_targets + 1;
	 ++compgoto_num_targets;
     }
     ((PU_PROFILE_HANDLE)pu_handle)->Compgoto_Profile_Table = Compgoto_Table;
  }
  IN_LIBINSTR = FALSE;
}

// Update appropriate profile information for an Compgoto
// Given a PU handle, rtrieve the Compgoto table; use this table 
// and the compgoto_id to retrieve the vector representing the
// possible targets for this Compgoto; target takes a 
// value between 0 and n-1 where n is the number of targets
// for the Compgoto. Increment the profile information for 
// the appropriate target represented by 'target'.
void
__profile_compgoto(void *pu_handle, INT32 compgoto_id, INT32 target, 
		 INT32 num_targets)
{
  Compgoto_Profile_Vector *Compgoto_Table = NULL;

  if (LIB_STATE != STARTED) return;
  if (IN_LIBINSTR == TRUE) return;
  IN_LIBINSTR = TRUE;

  Compgoto_Table = ((PU_PROFILE_HANDLE)pu_handle)->Compgoto_Profile_Table;

 if (target < 0 || target >= num_targets)
    target = -1;

  Compgoto_Table->data[compgoto_id].targets_profile[target +1]++;
  IN_LIBINSTR = FALSE;
}

// ====================================================================
// For each PU, we want to a one-time initialization of the 
// tables that maintain profile information of loop nodes. 
// Given a pu_handle and the number of loops in that PU, we 
// initialize the Loop_Table in the pu handle with the appropriate 
// number of entries. This routine may be invoked multiple times from 
// a PU, but the initialization is done only the first time.
void
__profile_loop_init(void *pu_handle, INT32 num_loops)
{

  LIBFB_Info_Loop_Vector *Loop_Table = NULL;

  if (LIB_STATE != STARTED) return;
  if (IN_LIBINSTR == TRUE) return;
  IN_LIBINSTR = TRUE;

  Loop_Table = ((PU_PROFILE_HANDLE)pu_handle)->LIBFB_Info_Loop_Table;
  if (Loop_Table == NULL) {
    Loop_Table = (LIBFB_Info_Loop_Vector *)MYMALLOC(sizeof(LIBFB_Info_Loop_Vector));
    Loop_Table->size = num_loops;
    Loop_Table->data = (LIBFB_Info_Loop*)MYMALLOC(num_loops * sizeof(LIBFB_Info_Loop));
    memset(Loop_Table->data, 0, num_loops * sizeof(LIBFB_Info_Loop));
    ((PU_PROFILE_HANDLE)pu_handle)->LIBFB_Info_Loop_Table = Loop_Table;
  }
  IN_LIBINSTR = FALSE;
}

// Update appropriate profile information at a loop entry.
void
__profile_loop(void *pu_handle, INT32 loop_id)
{
  LIBFB_Info_Loop_Vector *Loop_Table = NULL;

  if (LIB_STATE != STARTED) return;
  if (IN_LIBINSTR == TRUE) return;
  IN_LIBINSTR = TRUE;

  Loop_Table = ((PU_PROFILE_HANDLE)pu_handle)->LIBFB_Info_Loop_Table;
  if (Loop_Table->data[loop_id].invocation_count == 1) {
     Loop_Table->data[loop_id].min_trip_count = Loop_Table->data[loop_id].last_trip_count;
     Loop_Table->data[loop_id].max_trip_count = Loop_Table->data[loop_id].last_trip_count;
  } else if (Loop_Table->data[loop_id].invocation_count != 0) {
     Loop_Table->data[loop_id].min_trip_count = Min(Loop_Table->data[loop_id].min_trip_count,
				    Loop_Table->data[loop_id].last_trip_count);
     Loop_Table->data[loop_id].max_trip_count = Max(Loop_Table->data[loop_id].max_trip_count,
				    Loop_Table->data[loop_id].last_trip_count);
  }

  // Count num_zero_trips -- NOTE: The code does not check whether or not
  // the very last trip through the loop is a zero trip.  Instead, code
  // in procedure Convert_Loop_Profile of the file dump.cxx handles that
  // responsibility.
  if (Loop_Table->data[loop_id].invocation_count > 0 && Loop_Table->data[loop_id].last_trip_count == 0) {
     Loop_Table->data[loop_id].num_zero_trips++;
  }

  Loop_Table->data[loop_id].invocation_count++;
  Loop_Table->data[loop_id].last_trip_count = 0;
  IN_LIBINSTR = FALSE;
}

// Update appropriate profile information at a loop iteration.
void
__profile_loop_iter(void *pu_handle, INT32 loop_id)
{
  LIBFB_Info_Loop_Vector *Loop_Table = NULL;

  if (LIB_STATE != STARTED) return;
  if (IN_LIBINSTR == TRUE) return;
  IN_LIBINSTR = TRUE;

  Loop_Table = ((PU_PROFILE_HANDLE)pu_handle)->LIBFB_Info_Loop_Table;
  Loop_Table->data[loop_id].last_trip_count++;
  Loop_Table->data[loop_id].total_trip_count++;
  IN_LIBINSTR = FALSE;
}

// ====================================================================
// For each PU, we want to a one-time initialization of the 
// tables that maintain profile information of CAND/COR nodes. 
// Given a pu_handle and the number of CAND/COR in that PU, we 
// initialize the Short_Circuit_Table in the pu handle with the appropriate 
// number of entries. This routine may be invoked multiple times from 
// a PU, but the initialization is done only the first time.
void
__profile_short_circuit_init(void *pu_handle,
			     INT32 num_short_circuit_ops)
{
  LIBFB_Info_Circuit_Vector *Short_Circuit_Table = NULL;

  if (LIB_STATE != STARTED) return;
  if (IN_LIBINSTR == TRUE) return;
  IN_LIBINSTR = TRUE;

  Short_Circuit_Table = ((PU_PROFILE_HANDLE)pu_handle)->LIBFB_Info_Circuit_Table;
  if (Short_Circuit_Table == NULL) {
    Short_Circuit_Table = (LIBFB_Info_Circuit_Vector *)MYMALLOC(sizeof(LIBFB_Info_Circuit_Vector));
    Short_Circuit_Table->size = num_short_circuit_ops;
    Short_Circuit_Table->data = (LIBFB_Info_Circuit*)MYMALLOC(num_short_circuit_ops * sizeof(LIBFB_Info_Circuit));
    memset(Short_Circuit_Table->data, 0, num_short_circuit_ops * sizeof(LIBFB_Info_Circuit));
    ((PU_PROFILE_HANDLE)pu_handle)->LIBFB_Info_Circuit_Table = Short_Circuit_Table;
  }
  IN_LIBINSTR = FALSE;
}

// Update appropriate profile information for right operand of a CAND/COR
void
__profile_short_circuit(void *pu_handle, INT32 short_circuit_id,
		      BOOL taken)
{
  if (LIB_STATE != STARTED) return;
  if (IN_LIBINSTR == TRUE) return;
  IN_LIBINSTR = TRUE;

  if (taken) 
       Incr_Right_Taken(((PU_PROFILE_HANDLE)pu_handle), short_circuit_id);
  else 
       Incr_Neither_Taken(((PU_PROFILE_HANDLE)pu_handle), short_circuit_id);

  IN_LIBINSTR = FALSE;
}

// Given a PU handle, retrieve the CAND/COR table and increment
// right_taken_count for id.
static void Incr_Right_Taken(PU_PROFILE_HANDLE pu_handle, INT32 id)
{
  LIBFB_Info_Circuit_Vector *Short_Circuit_Table = NULL;
  Short_Circuit_Table = pu_handle->LIBFB_Info_Circuit_Table;
  Short_Circuit_Table->data[id].freq_right++;
}

// Given a PU handle, retrieve the CAND/COR table and increment
// neither_taken_count for id.
static void Incr_Neither_Taken(PU_PROFILE_HANDLE pu_handle, INT32 id)
{
  LIBFB_Info_Circuit_Vector *Short_Circuit_Table = NULL;
  Short_Circuit_Table = pu_handle->LIBFB_Info_Circuit_Table;
  Short_Circuit_Table->data[id].freq_neither++;
}

// ====================================================================
// For each PU, we want to a one-time initialization of the
// tables that maintain profile information of CALL nodes.
// Given a pu_handle and the number of CALL nodes in that PU, we
// initialize the Call_Profile_Table in the pu handle with the appropriate
// number of entries. This routine may be invoked multiple times from
// a PU, but the initialization is done only the first time.
void 
__profile_call_init(void *pu_handle, INT32 num_calls)
{
  LIBFB_Info_Call_Vector *Call_Table = NULL;

  if (LIB_STATE != STARTED) return;
  if (IN_LIBINSTR == TRUE) return;
  IN_LIBINSTR = TRUE;

  Call_Table = ((PU_PROFILE_HANDLE)pu_handle)->LIBFB_Info_Call_Table;
  if (Call_Table == NULL) {
    Call_Table = (LIBFB_Info_Call_Vector *)MYMALLOC(sizeof(LIBFB_Info_Call_Vector));
    Call_Table->size = num_calls;
    Call_Table->data = (LIBFB_Info_Call*)MYMALLOC(num_calls * sizeof(LIBFB_Info_Call));
    memset(Call_Table->data, 0, num_calls * sizeof(LIBFB_Info_Call));
    ((PU_PROFILE_HANDLE)pu_handle)->LIBFB_Info_Call_Table = Call_Table;
  }
  IN_LIBINSTR = FALSE;
}

// Update entry count for a call
void 
__profile_call_entry(void *pu_handle, INT32 call_id)
{
  LIBFB_Info_Call_Vector *Call_Table = NULL;

  if (LIB_STATE != STARTED) return;
  if (IN_LIBINSTR == TRUE) return;
  IN_LIBINSTR = TRUE;

  Call_Table = ((PU_PROFILE_HANDLE)pu_handle)->LIBFB_Info_Call_Table;
  Call_Table->data[call_id].freq_entry++;

  IN_LIBINSTR = FALSE;
}

// Update exit count for a call
void 
__profile_call_exit(void *pu_handle, INT32 call_id)
{
  LIBFB_Info_Call_Vector *Call_Table = NULL;

  if (LIB_STATE != STARTED) return;
  if (IN_LIBINSTR == TRUE) return;
  IN_LIBINSTR = TRUE;

  Call_Table = ((PU_PROFILE_HANDLE)pu_handle)->LIBFB_Info_Call_Table;
  Call_Table->data[call_id].freq_exit++;
  IN_LIBINSTR = FALSE;
}

static void
Set_Instrumentation_Phase_Num(PROFILE_PHASE phase_num)
{
  instrumentation_phase_num = phase_num;
}

// ====================================================================

extern PROFILE_PHASE
Instrumentation_Phase_Num()
{
  return instrumentation_phase_num;
}

// ====================================================================

// ====================================================================

#define UNDEFINED -1
// One time initialization
void __profile_init(char *fname, int phase_num, BOOL unique_name)
{
  PROFILE_PHASE curr_phase_num;
  void (*pf)() = __profile_finish;

  if (LIB_STATE != TO_BE_STARTED) return;
  if (IN_LIBINSTR == TRUE) return;
  IN_LIBINSTR = TRUE;
  LIB_STATE = START_IN_PROGRESS;

  atexit(pf);
  output_filename = (char*)MYMALLOC(strlen(fname) + 7 + 1);
  strcpy(output_filename, fname); 
  if (unique_name)
    strcat (output_filename, ".XXXXXX");
  unique_output_filename = unique_name;
  PU_Profile_Handle_Table = hash_new (64, sizeof(PU_PROFILE_HANDLE));

  curr_phase_num = Instrumentation_Phase_Num();

  if (curr_phase_num == PROFILE_PHASE_NONE) {
    Set_Instrumentation_Phase_Num((PROFILE_PHASE) phase_num);
  } else if(curr_phase_num != (PROFILE_PHASE) phase_num) {
    profile_warn("Phase Number already set to a different value in: %s",
		 output_filename);
  }
  LIB_STATE = STARTED;
  IN_LIBINSTR = FALSE;
}


// PU level initialization to gather profile information for the PU.
// We call atexit during the first call to this routine to ensure
// that at exit we remember to destroy the data structures and dump
// profile information.
// Also, during the first call, a profile handle for the PU is created.
// During subsequent calls, teh PC address of the PU is used to access
// a hash table and return the profile handle that was created during the 
// first call.

void *
__profile_pu_init(char *file_name, char* pu_name, long current_pc,
		  INT32 checksum)
{
  hash_bucket_t bucket;
  PU_PROFILE_HANDLE *data=NULL;

  if (LIB_STATE != STARTED) return NULL;
  if (IN_LIBINSTR == TRUE) return NULL;
  IN_LIBINSTR = TRUE;
  bucket = hash_access_bucket(PU_Profile_Handle_Table, (const char *)&current_pc, sizeof(long));
  if (bucket == NULL) {
    long *key = MYMALLOC(sizeof(long));
    *key = current_pc;
    bucket = hash_insert_bucket(PU_Profile_Handle_Table, (const char *)key, sizeof(long));
    data = (PU_PROFILE_HANDLE*)hash_bucket_data(bucket);
    *data = PU_Profile_Handle_new(file_name, pu_name, checksum);
  } else {
    data = (PU_PROFILE_HANDLE *)hash_bucket_data(bucket);
  }
  IN_LIBINSTR = FALSE;
  return (void *) *data;
}


// At exit processing to destroy data structures and dump profile
// information.

void __profile_finish(void)
{
  FILE *fp;
  hash_iterator_t iterator;
  extern int mkstemp(char *);

  if (LIB_STATE != STARTED) return;
  if (IN_LIBINSTR == TRUE) return;
  IN_LIBINSTR = TRUE;
  LIB_STATE = FINISHED_IN_PROGRESS;
/*   if (unique_output_filename)  */
  {
      int file_id = mkstemp (output_filename);
      fp = fdopen (file_id, "w+");
  }
/*    else */
/*       fp = fopen (output_filename, "w+"); */

  if (fp == NULL) {
     profile_error("Unable to open file: %s", output_filename);
     IN_LIBINSTR = FALSE;
     return;
  } 

  Dump_all(fp, output_filename);
  
  /* Free allcated memory*/
  for(iterator = hash_iterator_new(PU_Profile_Handle_Table); 
      !hash_iterator_at_end(iterator); 
      hash_iterator_advance(iterator)) {
    hash_bucket_t bucket = hash_iterator_current(iterator);
    PU_PROFILE_HANDLE *pu_handle = (PU_PROFILE_HANDLE*)hash_bucket_data(bucket);
    long *key = (long*)hash_bucket_key(bucket);
    MYFREE(key);
    PU_Profile_Handle_del(*pu_handle);
  }
  iterator = hash_iterator_del(iterator);

  PU_Profile_Handle_Table = hash_del(PU_Profile_Handle_Table);

  fclose(fp);

  MYFREE(output_filename);
  output_filename = NULL;
  LIB_STATE = FINISHED;
  IN_LIBINSTR = FALSE;
}

void __profile_on(void)
{
  LIB_STATE = SAVE_LIB_STATE;
}

void __profile_off(void)
{
  SAVE_LIB_STATE = LIB_STATE;
  LIB_STATE = OFF;
}
