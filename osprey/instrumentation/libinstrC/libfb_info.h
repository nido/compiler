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
typedef signed long long mINT64;
typedef int		BOOL;	/* Natural size Boolean value */
typedef unsigned long long mUINT64;
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
  PROFILE_PHASE_LAST		= 4,
  PROFILE_PHASE_MAX             = 64  // Fb_Hdr size must be 0 mod 64
} PROFILE_PHASE;


/* Feedback File Format */

#define FB_NIDENT       16

#define INSTR_MAG          "\177INS"

#define INSTR_CURRENT      1

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

} Fb_Hdr; 

typedef struct Pu_Hdr {
  INT32 pu_checksum;
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
} Pu_Hdr;


// data structures shared by both the instrumentation runtime and the back
// end. 

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

#endif //libfb_info_INCLUDED
