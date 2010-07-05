/*

  Copyright (C) 2000, 2001 Silicon Graphics, Inc.  All Rights Reserved.

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

  Contact information:  Silicon Graphics, Inc., 1600 Amphitheatre Pky,
  Mountain View, CA 94043, or:

  http://www.sgi.com

  For further information regarding this notice, see:

  http://oss.sgi.com/projects/GenInfo/NoticeExplan

*/


//-*-c++-*-
// ====================================================================
// ====================================================================
//
// Module: fb_info.h
//
// Description:
//
// During instrumentation, certain events associated with program
// control flow (such as the number of times a particular branch is
// taken, or not taken) are counted.  During subsequent compilations,
// this data may be retrieved and used to guide optimization decisions.
//
// fb_info.h defines data structs used to store this feedback data for
// various types of whirl nodes.  These structures are used by the
// FEEDBACK class (defined in fb_whirl.h) and by the instrumentation runtime
//
// ====================================================================
// ====================================================================

#ifndef libfb_info_INCLUDED
#define libfb_info_INCLUDED

typedef unsigned int  ULONG;
typedef unsigned int  UINT32;
typedef signed int	INT32;	/* The natural integer matches */
typedef signed long long INT64;	
typedef unsigned long long UINT64;	
typedef signed long long mINT64;
typedef int		BOOL;	/* Natural size Boolean value */
typedef unsigned long long mUINT64;
/* Define pointer-sized integers for the host machine: */
typedef signed long	INTPS;	/* Pointer-sized integer */
typedef unsigned long	UINTPS;	/* Pointer-sized integer */
/* Define standard values: */
#ifndef TRUE
#define TRUE	((BOOL) 1)
#endif
#ifndef FALSE
#define FALSE	((BOOL) 0)
#endif

// When to instrument?  Correlates to Instrumentation_Phase_Num
typedef enum PROFILE_PHASE
{
  PROFILE_PHASE_NONE            = -1,
  PROFILE_PHASE_BEFORE_VHO	= 0,
  PROFILE_PHASE_IPA_CUTOFF	= 0,	// phases less than or equal to
					// IPA_CUTOFF will not be
					// instrumented when the input file 
					// is an ipa-generated file.
  PROFILE_PHASE_BEFORE_LNO	= 1,
  PROFILE_PHASE_BEFORE_WOPT	= 2,
  PROFILE_PHASE_BEFORE_CG	= 3,
  PROFILE_PHASE_BEFORE_REGION	= 4,
  PROFILE_PHASE_LAST		= 5,
  PROFILE_PHASE_MAX             = 64  // Fb_Hdr size must be 0 mod 64
} PROFILE_PHASE;


// What instrument? Correlates to Profile_Type
typedef enum PROFILE_TYPE
{
  WHIRL_PROFILE     = 1,
  CG_EDGE_PROFILE   = 2,
  CG_VALUE_PROFILE  = 4,
  CG_STRIDE_PROFILE  = 8,
  PROFILE_TYPE_LAST = 16,
  PROFILE_TYPE_MAX  = 64
} PROFILE_TYPE;


/* Feedback File Format */

#define FB_NIDENT       16

#define INSTR_MAG          "\177INS"

#define INSTR_CURRENT      2

#ifdef __cplusplus
#include <string.h> // For memcpy
#endif
typedef struct Fb_Hdr {
  char fb_ident[FB_NIDENT];     /* ident bytes */
  ULONG fb_endianness;              /* 0 is little endian, else is big */
  ULONG fb_version;             /* file version */
  ULONG fb_profile_offset;	/* file offset for profile data */
  ULONG fb_pu_hdr_offset;       /* PU header file offset */
  ULONG fb_pu_hdr_ent_size;     /* PU header entry size */ 
  ULONG fb_pu_hdr_num;          /* Number of PU header entries */
  ULONG fb_str_table_offset;
  ULONG fb_str_table_size;
  PROFILE_PHASE phase_num;

#ifdef __cplusplus
  Fb_Hdr() {}
  Fb_Hdr(Fb_Hdr& x) 
  {
    memcpy((void *)fb_ident,(void *)x.fb_ident, FB_NIDENT);
    fb_version = x.fb_version;
    fb_profile_offset = x.fb_profile_offset;
    fb_pu_hdr_offset = x.fb_pu_hdr_offset;
    fb_pu_hdr_ent_size = x.fb_pu_hdr_ent_size;
    fb_pu_hdr_num = x.fb_pu_hdr_num;
    fb_str_table_offset = x.fb_str_table_offset;
    fb_str_table_size = x.fb_str_table_size;
    phase_num = x.phase_num;
  }
  void Print( FILE *fp ) const {
  	fprintf(fp, "\n**********      FILE HEADER     **************\n");
  	fprintf(fp, "fb_ident = %s\n", fb_ident);
  	fprintf(fp, "fb_endianness = %u\n",fb_endianness);
  	fprintf(fp, "fb_version = %u\n",fb_version);
  	fprintf(fp, "fb_profile_offset = %u\n",fb_profile_offset);
  	fprintf(fp, "fb_pu_hdr_offset = %u\n",fb_pu_hdr_offset);
  	fprintf(fp, "fb_pu_hdr_ent_size = %u\n",fb_pu_hdr_ent_size);
  	fprintf(fp, "fb_pu_hdr_num = %u\n",fb_pu_hdr_num);
  	fprintf(fp, "fb_str_table_offset = %u\n",fb_str_table_offset);
  	fprintf(fp, "fb_str_table_size = %u\n",fb_str_table_size);
  	fprintf(fp, "phase_num = %u\n",phase_num);
  };
#endif
} Fb_Hdr; 

typedef struct Pu_Hdr {
  INT32 pu_checksum;
  INT32 pu_size;
  UINT64 runtime_fun_address;
  ULONG pu_name_index;
  ULONG pu_file_offset;
  ULONG pu_inv_offset;
  ULONG pu_num_inv_entries;
  ULONG pu_br_offset;
  ULONG pu_num_br_entries;
  ULONG pu_switch_offset;
  ULONG pu_switch_target_offset;	// # of targets for each swtich stmt
  ULONG pu_num_switch_entries;
  ULONG pu_cgoto_offset;
  ULONG pu_cgoto_target_offset;		// # of targets for each compgoto
  ULONG pu_num_cgoto_entries;
  ULONG pu_loop_offset;
  ULONG pu_num_loop_entries;
  ULONG pu_scircuit_offset;
  ULONG pu_num_scircuit_entries;
  ULONG pu_call_offset;
  ULONG pu_num_call_entries;
#ifdef KEY
  ULONG pu_value_offset;
  ULONG pu_num_value_entries;
  ULONG pu_value_fp_bin_offset;
  ULONG pu_num_value_fp_bin_entries;
#endif

  ULONG pu_icall_offset;
  ULONG pu_num_icall_entries;
  ULONG pu_handle;
  ULONG pu_edge_offset;
  ULONG pu_num_edge_entries;
  ULONG pu_instr_count;
  ULONG pu_instr_exec_count;
#ifdef KEY
  ULONG pu_values_offset;
  ULONG pu_values_fp_bin_offset;
#endif
  ULONG pu_ld_count;   //prefetch count
  ULONG pu_stride_offset;
#if 1 // [Tb] For st200 targets no need to add padding.
  // KEY
  // This header file will be compiled into the 64-bit instrumentation library
  // as well as the 32-bit compiler. So, we should pad it to a 8-byte boundary.
  ULONG pad;
#endif
  
#ifdef __cplusplus
  Pu_Hdr() {
    pu_size=54321;
    pu_handle=11111;
    pu_edge_offset=222;
    pu_num_edge_entries=0;
    pu_instr_count=0;
    pu_instr_exec_count=0;
    pu_icall_offset=333;
    pu_num_icall_entries=0;
#ifdef KEY
    pu_values_offset=444;
    pu_values_fp_bin_offset=666;
#else
    pu_value_offset=444;
#endif
    pu_ld_count=0;
    pu_stride_offset=555;
#if 0
    pad = 0;
#endif
  }

  void Print( FILE * fp, int id=-1) const {
  	fprintf(fp, "\n**********   PU Header No %d   **************\n", id);
  	fprintf(fp, "pu_checksum = %d\n", pu_checksum);
  	fprintf(fp, "pu_size = %d\n", pu_size);
  	fprintf(fp, "runtime_fun_address= %llu\n", runtime_fun_address);
  	fprintf(fp, "pu_name_index = %u\n", pu_name_index);
  	fprintf(fp, "pu_file_offset = %u\n", pu_file_offset);
  	fprintf(fp, "pu_inv_offset = %u\n", pu_inv_offset);
  	fprintf(fp, "pu_num_inv_entries = %u\n", pu_num_inv_entries);
  	fprintf(fp, "pu_br_offset = %u\n", pu_br_offset);
  	fprintf(fp, "pu_num_br_entries = %u\n", pu_num_br_entries);
  	fprintf(fp, "pu_switch_offset = %u\n", pu_switch_offset);
  	fprintf(fp, "pu_switch_target_offset = %u\n", pu_switch_target_offset);
  	fprintf(fp, "pu_num_switch_entries = %u\n", pu_num_switch_entries);
  	fprintf(fp, "pu_cgoto_offset = %u\n", pu_cgoto_offset);
  	fprintf(fp, "pu_cgoto_target_offset = %u\n", pu_cgoto_target_offset);
  	fprintf(fp, "pu_num_cgoto_entries = %u\n", pu_num_cgoto_entries);
  	fprintf(fp, "pu_loop_offset = %u\n", pu_loop_offset);
  	fprintf(fp, "pu_num_loop_entries = %u\n", pu_num_loop_entries);
  	fprintf(fp, "pu_scircuit_offset = %u\n", pu_scircuit_offset);
  	fprintf(fp, "pu_num_scircuit_entries = %u\n", pu_num_scircuit_entries);
  	fprintf(fp, "pu_call_offset = %u\n", pu_call_offset);
  	fprintf(fp, "pu_num_call_entries = %u\n", pu_num_call_entries);
#ifdef KEY
  	fprintf(fp, "pu_value_offset = %u\n",      pu_value_offset);
  	fprintf(fp, "pu_num_value_entries = %u\n", pu_num_value_entries);
  	fprintf(fp, "pu_value_fp_bin_offset = %u\n",  pu_value_fp_bin_offset);
  	fprintf(fp, "pu_num_value_fp_bin_entries = %u\n", 
		pu_num_value_fp_bin_entries);
#endif
  	fprintf(fp, "pu_icall_offset = %u\n", pu_icall_offset);
  	fprintf(fp, "pu_num_icall_entries = %u\n", pu_num_icall_entries);
  	fprintf(fp, "pu_handle = %u\n", pu_handle);
  	fprintf(fp, "pu_edge_offset = %u\n", pu_edge_offset);
  	fprintf(fp, "pu_num_edge_entries = %u\n", pu_num_edge_entries);
  	fprintf(fp, "pu_instr_count = %u\n", pu_instr_count);
  	fprintf(fp, "pu_instr_exec_count = %u\n", pu_instr_exec_count);
#ifdef KEY
  	fprintf(fp, "pu_values_offset = %u\n", pu_values_offset);
  	fprintf(fp, "pu_values_fp_bin_offset = %u\n", pu_values_fp_bin_offset);
#else
  	fprintf(fp, "pu_value_offset = %u\n", pu_value_offset);
#endif
  	fprintf(fp, "pu_ld_count = %u\n", pu_ld_count);
  	fprintf(fp, "pu_stride_offset = %u\n", pu_stride_offset);
  };
#endif // __cplusplus
}Pu_Hdr;

#ifndef __cplusplus
#ifdef INSTR_DEBUG
static inline void Pu_Hdr_Print( Pu_Hdr *this, FILE * fp)  {
  fprintf(fp, "\n**********   PU Header   **************\n");
  fprintf(fp, "pu_checksum = %d\n", this->pu_checksum);
  fprintf(fp, "pu_size = %d\n", this->pu_size);
  fprintf(fp, "runtime_fun_address= 0x%llx\n", this->runtime_fun_address);
  fprintf(fp, "pu_name_index = %u\n", this->pu_name_index);
  fprintf(fp, "pu_file_offset = %u\n", this->pu_file_offset);
  fprintf(fp, "pu_inv_offset = %u\n", this->pu_inv_offset);
  fprintf(fp, "pu_num_inv_entries = %u\n", this->pu_num_inv_entries);
  fprintf(fp, "pu_br_offset = %u\n", this->pu_br_offset);
  fprintf(fp, "pu_num_br_entries = %u\n", this->pu_num_br_entries);
  fprintf(fp, "pu_switch_offset = %u\n", this->pu_switch_offset);
  fprintf(fp, "pu_switch_target_offset = %u\n", this->pu_switch_target_offset);
  fprintf(fp, "pu_num_switch_entries = %u\n", this->pu_num_switch_entries);
  fprintf(fp, "pu_cgoto_offset = %u\n", this->pu_cgoto_offset);
  fprintf(fp, "pu_cgoto_target_offset = %u\n", this->pu_cgoto_target_offset);
  fprintf(fp, "pu_num_cgoto_entries = %u\n", this->pu_num_cgoto_entries);
  fprintf(fp, "pu_loop_offset = %u\n", this->pu_loop_offset);
  fprintf(fp, "pu_num_loop_entries = %u\n", this->pu_num_loop_entries);
  fprintf(fp, "pu_scircuit_offset = %u\n", this->pu_scircuit_offset);
  fprintf(fp, "pu_num_scircuit_entries = %u\n", this->pu_num_scircuit_entries);
  fprintf(fp, "pu_call_offset = %u\n", this->pu_call_offset);
  fprintf(fp, "pu_num_call_entries = %u\n", this->pu_num_call_entries);
  fprintf(fp, "pu_value_offset = %u\n", this->     pu_value_offset);
  fprintf(fp, "pu_num_value_entries = %u\n", this->pu_num_value_entries);
  fprintf(fp, "pu_value_fp_bin_offset = %u\n", this-> pu_value_fp_bin_offset);
  fprintf(fp, "pu_num_value_fp_bin_entries = %u\n", this->pu_num_value_fp_bin_entries);
  fprintf(fp, "pu_icall_offset = %u\n", this->pu_icall_offset);
  fprintf(fp, "pu_num_icall_entries = %u\n", this->pu_num_icall_entries);
  fprintf(fp, "pu_handle = %u\n", this->pu_handle);
  fprintf(fp, "pu_edge_offset = %u\n", this->pu_edge_offset);
  fprintf(fp, "pu_num_edge_entries = %u\n", this->pu_num_edge_entries);
  fprintf(fp, "pu_instr_count = %u\n", this->pu_instr_count);
  fprintf(fp, "pu_instr_exec_count = %u\n", this->pu_instr_exec_count);
  fprintf(fp, "pu_values_offset = %u\n", this->pu_values_offset);
  fprintf(fp, "pu_values_fp_bin_offset = %u\n", this->pu_values_fp_bin_offset);
  fprintf(fp, "pu_ld_count = %u\n", this->pu_ld_count);
  fprintf(fp, "pu_stride_offset = %u\n", this->pu_stride_offset);
};
#endif
#endif

// data structures use only by the libinstrC. 

typedef struct LIBFB_Info_Invoke {
  INT64 freq_invoke;    // number of times statement invoked
} LIBFB_Info_Invoke;

typedef struct LIBFB_Info_Branch {

  INT64 freq_taken;     // number of times branch taken
                          // (then clause for IF, Kid0 for CSELECT)
  INT64 freq_not_taken; // number of times branch not taken
                          // (else clause for IF, Kid1 for CSELECT)
}LIBFB_Info_Branch;

/* // ==================================================================== */

/* // Feedback info for a loop node */
/* //         ___|__ */
/* //        | test | */
/* //        |______| */
/* //          /  \ */
/* //     zero/N  Y\positive */
/* //        /      \    ____ */
/* //        |      _\__/_   \ */
/* //        |     | body |   | */
/* //        |     |______|   |        exit    = zero     + out */
/* //        |     | test |   |        iterate = positive + back */
/* //        |     |______|   | */
/* //        |       /  \    /         In a DO_WHILE loop, zero = 0. */
/* //        |   out/N  Y\__/ */
/* //         \    /     back */
/* //         _\__/_ */
/* //        |      | */


typedef struct LIBFB_Info_Loop {
   INT64 invocation_count; // total times the loop is invoked
   INT64 total_trip_count; // total trip count from all invocations
   INT64 last_trip_count;  // trip count from last invocation
   INT64 min_trip_count;   // minimum trip count from previous invocations
   INT64 max_trip_count;   // maximum trip count from previous invocations
   INT64 num_zero_trips;   // Number of times the loop had a zero trip
} LIBFB_Info_Loop;

typedef struct LIBFB_Info_Circuit {

  INT64 freq_left;   // "taken" means false for CAND, true for CIOR
  INT64 freq_right;
  INT64 freq_neither;

}LIBFB_Info_Circuit;

typedef struct LIBFB_Info_Call {
  INT64 freq_entry;
  INT64 freq_exit;
}LIBFB_Info_Call;

typedef struct LIBFB_Info_Switch {
   INT64 *freq_targets;
}LIBFB_Info_Switch;


// Be carefull... if you change these values update also
// common/com/fb_tnv.h and common/com/fb_info.h!!!
#define TNV 10
#define FB_TNV_SIZE  10

typedef struct LIBFB_Info_Value {
  INT64   num_values;   // how many valid entries in the value array
  INT64 exe_counter;  // how many times this inst is executed
  INT64   value[TNV];   // the top TNV profiled values
  INT64 freq[TNV];    // the corresponding freq for each value
}LIBFB_Info_Value;

typedef struct LIBFB_Info_Value_FP_Bin {
  INT64 exe_counter;  // how many times this inst is executed
  INT64 zopnd0;       // how many times was operand 0 == 0.0
  INT64 zopnd1;       // how many times was operand 1 == 0.0
  INT64 uopnd0;       // how many times was operand 0 == 1.0
  INT64 uopnd1;       // how many times was operand 1 == 1.0
}LIBFB_Info_Value_FP_Bin;

typedef struct LIBFB_TNV{
  UINT32 _id; // instruction id.
  INT32 _flag; //0 for integer type, 1 for float type.
  //Note: for "float" type, we just put it here as an "integer". Because they have same size.
  UINT64 _address; //record previous address
  UINT64 _exec_counter; // how many times does this instruction executed.
  UINT64 _clear_counter;
  UINT64 _sample_counter; //do stride profile sample 
  UINT64 _stride_steps;
  UINT64 _zero_std_counter;
  UINT64 _values[FB_TNV_SIZE]; //top 10 values. 
  UINT64 _counters[FB_TNV_SIZE]; //counters for top 10 values.
} LIBFB_TNV;

typedef struct LIBFB_Info_Icall{
  LIBFB_TNV tnv;
}LIBFB_Info_Icall;

typedef struct LIBFB_Info_Edge {
  INT64 freq_edge;    // number of times edge
}LIBFB_Info_Edge;

typedef struct LIBFB_Info_Stride{
      LIBFB_TNV tnv;
}LIBFB_Info_Stride;


#endif //libfb_info_INCLUDED
