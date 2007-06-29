//-*-c++-*-
// ====================================================================
// ====================================================================
//
// Module: profile_aux.h
//
// ====================================================================
//
// Description:
//
// These data structures are used to store frequency counts during
// the run of an instrumented program unit.
//
// ====================================================================
// ====================================================================


#ifndef profile_aux_INCLUDED
#define profile_aux_INCLUDED
#include "libfb_info.h"
#include "hash.h"
#include "profile_error.h"
// ====================================================================

typedef enum Type_Profile {
  Invoke_Type_Profile,
  Branch_Type_Profile,
  Compgoto_Type_Profile,
  Short_Circuit_Type_Profile,
  Call_Type_Profile,
  Loop_Type_Profile,
  Switch_Type_Profile,
  Icall_Type_Profile,
  Value_Type_Profile,
  Value_FP_Bin_Type_Profile
}Type_Profile;

// All the information about an invoke node is stored in the
// Invoke_Profile structure. Currently for each invokation
// we maintain information on the number of time the invokation
// occured


// All the information about a branch node is stored in the
// Branch_Profile structure. Currently for each conditional
// branch we maintain information on whether the branch was 
// taken or not


// All the information about an Compgoto node is maintained
// in the Compgoto_Profile structure. For each possible jump
// target of an Compgoto, we want to gather information on how
// many times we branched to that target from this Compgoto.

// The data member targets_profile is a vector; the ith element
// of this vector gives a count of how many times we jumped to
// target i of the Compgoto.

typedef struct Compgoto_Profile {
  INT64 *targets_profile;
  size_t targets_profile_size;
} Compgoto_Profile;
	
// All the information about a CAND and COR is stored
// in the Short_Circuit_Profile structure.

// All the information about a CALL is stored in 
// Call_Profile structure.

// All the information about a DO LOOP or WHILE LOOP is stored
// in the LIBFB_Info_Loop structure.

// All the information about an Switch node is maintained
// in the Switch_Profile structure. For each possible jump
// target of a Switch, we want to gather information on how
// many times we branched to that target from this Switch.

// The ith element of the vector targets_profile counts how many
// times we jump to target i of the Switch.
// The ith element of the vector targets_case_value records the
// index value on which the ith branch is taken.
// Note that   targets_profile.size () == targets_case_value.size () + 1

// The 2 first fields of this must be the same as for Compgoto_Profile struct.
typedef struct Switch_Profile {
  INT64 *targets_profile;
  size_t targets_profile_size;
  INT64 *targets_case_value;
  //  size_t targets_case_value_size is targets_profile_size -1;
}Switch_Profile;

// ====================================================================

// For each PU, we maintain a handle that can be retrieved from
// a hash table using the PC address of the PU. This handle 
// contains pointers to all the profile information for that PU.
// Currently, we only have a Branch_Profile_Table and a
// Compgoto_Profile_Table in this handle to keep profile information
// of conditional branches and Compgotos. The handle also contains the
// file name and the name of the PU which are set when the handle is
// first created.

typedef struct LIBFB_Info_Invoke_Vector {
  LIBFB_Info_Invoke* data; 
  size_t size;
} LIBFB_Info_Invoke_Vector;

typedef struct LIBFB_Info_Branch_Vector {
  LIBFB_Info_Branch* data; 
  size_t size;
} LIBFB_Info_Branch_Vector;

typedef struct Switch_Profile_Vector {
  Switch_Profile* data; 
  size_t size;
} Switch_Profile_Vector;

typedef struct Compgoto_Profile_Vector {
  Compgoto_Profile* data; 
  size_t size;
} Compgoto_Profile_Vector;

typedef struct LIBFB_Info_Loop_Vector {
  LIBFB_Info_Loop* data; 
  size_t size;
} LIBFB_Info_Loop_Vector;

typedef struct LIBFB_Info_Circuit_Vector {
  LIBFB_Info_Circuit* data; 
  size_t size;
} LIBFB_Info_Circuit_Vector;

typedef struct LIBFB_Info_Call_Vector {
  LIBFB_Info_Call* data; 
  size_t size;
} LIBFB_Info_Call_Vector;

typedef struct LIBFB_Info_Icall_Vector {
  LIBFB_Info_Icall* data; 
  size_t size;
} LIBFB_Info_Icall_Vector;

#ifdef KEY
typedef struct LIBFB_Info_Value_Vector {
  LIBFB_Info_Value* data; 
  size_t size;
} LIBFB_Info_Value_Vector;

typedef struct LIBFB_Info_Value_FP_Bin_Vector {
  LIBFB_Info_Value_FP_Bin* data; 
  size_t size;
} LIBFB_Info_Value_FP_Bin_Vector;

#endif
typedef struct PU_Profile_Handle {
    LIBFB_Info_Invoke_Vector	*LIBFB_Info_Invoke_Table;
    LIBFB_Info_Branch_Vector	*LIBFB_Info_Branch_Table;
    Switch_Profile_Vector	*Switch_Profile_Table;
    Compgoto_Profile_Vector	*Compgoto_Profile_Table;
    LIBFB_Info_Loop_Vector		*LIBFB_Info_Loop_Table;
    LIBFB_Info_Circuit_Vector *LIBFB_Info_Circuit_Table;
    LIBFB_Info_Call_Vector		*LIBFB_Info_Call_Table;
    LIBFB_Info_Icall_Vector		*LIBFB_Info_Icall_Table;
#ifdef KEY
    LIBFB_Info_Value_Vector		*LIBFB_Info_Value_Table;
    LIBFB_Info_Value_FP_Bin_Vector	*LIBFB_Info_Value_FP_Bin_Table;
#endif

    INT32 checksum;
    char *file_name;
    char *pu_name;
    INT32 pu_size;
    UINT64 runtime_fun_address;
}PU_Profile_Handle;

typedef struct PU_Profile_Handle * PU_PROFILE_HANDLE;

typedef hash_t HASH_MAP;

extern HASH_MAP PU_Profile_Handle_Table;

#endif /* profile_aux_INCLUDED */
