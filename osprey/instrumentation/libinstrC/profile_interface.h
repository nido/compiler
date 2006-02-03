//-*-c++-*-
// ====================================================================
// ====================================================================
//
// Module: profile_interface.h
//
// ====================================================================
//
// Copyright (C) 2000, 2001 Silicon Graphics, Inc.  All Rights Reserved.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of version 2 of the GNU General Public License as
// published by the Free Software Foundation.
//
// This program is distributed in the hope that it would be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//
// Further, this software is distributed without any warranty that it
// is free of the rightful claim of any third person regarding
// infringement  or the like.  Any license provided herein, whether
// implied or otherwise, applies only to this software file.  Patent
// licenses, if any, provided herein do not apply to combinations of
// this program with other software, or any other product whatsoever.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write the Free Software Foundation,
// Inc., 59 Temple Place - Suite 330, Boston MA 02111-1307, USA.
//
// Contact information:  Silicon Graphics, Inc., 1600 Amphitheatre Pky,
// Mountain View, CA 94043, or:
//
// http://www.sgi.com
//
// For further information regarding this notice, see:
//
// http://oss.sgi.com/projects/GenInfo/NoticeExplan
//
// ====================================================================
//
// Description:
//
// During instrumentation, calls to the following procedures are
// inserted into the WHIRL code.  When invoked, these procedures
// initialize, perform, and finalize frequency counts.
//
// ====================================================================
// ====================================================================


#ifndef profile_interface_INCLUDED
#define profile_interface_INCLUDED

#include "libfb_info.h"

// The calls to these routines are generated by the compiler during
// instrumentation. Since the names have to match the compiler
// generated names, we need extern C to prevent name mangling.

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

// ====================================================================


// One time initialization

extern void __profile_init(char *output_filename, int phase_num,
			   BOOL unique_name); 


// PU level initializations

extern void * __profile_pu_init(char *file_name, char* pu_name,
				long current_pc, INT32 checksum);


// Profile routines for invokations

extern void __profile_invoke_init(void *pu_handle, INT32 num_invokes);
extern void __profile_invoke(void *pu_handle, INT32 invoke_id);


// Profile routines for conditional branches

extern void __profile_branch_init(void *pu_handle, INT32 num_branches);
extern void __profile_branch(void *pu_handle, INT32 id, BOOL taken);


// Profile routines for switches

extern void __profile_switch_init(void *pu_handle,
				  INT32 num_switch,      INT32 *num_targets,
				  INT32 num_case_values, INT64 *case_values);
extern void __profile_switch(void *pu_handle, INT32 switch_id,
			     INT32 target, INT32 num_targets);


// Profile routines for compgotos

extern void __profile_compgoto_init(void *pu_handle, int num_compgoto,
				    int *num_targets);
extern void __profile_compgoto(void *pu_handle, INT32 compgoto_id,
			       INT32 target, INT32 num_targets);


// Profile routines for loops

extern void __profile_loop_init(void *pu_handle, int num_loops);
extern void __profile_loop(void *pu_handle, INT32 id);
extern void __profile_loop_iter(void *pu_handle, INT32 id);


// Profile routines for short circuiting

extern void __profile_short_circuit_init(void *pu_handle,
					 int num_short_circuit_ops);
extern void __profile_short_circuit(void *pu_handle, INT32 short_circuit_id,
				    BOOL taken);


// Profile routines for calls

extern void __profile_call_init(void *pu_handle, int num_calls);
extern void __profile_call_entry(void *pu_handle, int call_id);
extern void __profile_call_exit(void *pu_handle, int call_id);


// PU level cleanup 

extern void __profile_finish(void);

// to put ON and Off instrumentation
extern void __profile_on(void);
  extern void __profile_off(void);



// ====================================================================


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* profile_interface_INCLUDED */
