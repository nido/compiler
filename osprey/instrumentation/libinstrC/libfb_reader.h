// ====================================================================
//
// Description:
//
// Propose conversion reader from little endian instrumentation file
// to big endian
// 
//
// ====================================================================
// ====================================================================

#ifndef libfb_reader_INCLUDED
#define libfb_reader_INCLUDED

#include <stdio.h>
#include "libfb_info.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
  extern void fb_reader_INT64_list_Read(INT64 *this_c, int nitems, FILE *fp, 
					  char *output_filename);
  // Read the main header in the feedback file
  extern void fb_reader_Fb_Hdr_Read(Fb_Hdr *this_c, FILE *fp, char *output_filename);

  extern void fb_reader_Pu_Hdr_list_Read(Pu_Hdr *this_c, Fb_Hdr *fb_hdr,
					 FILE *fp, char *output_filename);

  extern void fb_reader_Info_Invoke_list_Read(LIBFB_Info_Invoke *this_c, Pu_Hdr* pu_hdr_entry, long pu_ofst,
						 FILE *fp, char *fname);

  extern void fb_reader_Info_Branch_list_Read(LIBFB_Info_Branch *this_c, Pu_Hdr* pu_hdr_entry, long pu_ofst,
						 FILE *fp, char *fname);
  extern void fb_reader_Info_Loop_list_Read(LIBFB_Info_Loop *this_c, Pu_Hdr* pu_hdr_entry, long pu_ofst,
					       FILE *fp, char *fname);
  extern void fb_reader_Info_Circuit_list_Read(LIBFB_Info_Circuit *this_c, Pu_Hdr* pu_hdr_entry, long pu_ofst,
						  FILE *fp, char *fname);
  extern void fb_reader_Info_Call_list_Read(LIBFB_Info_Call *this_c, Pu_Hdr* pu_hdr_entry, long pu_ofst,
					       FILE *fp, char *fname);
  extern void fb_reader_Info_Icall_list_Read(LIBFB_Info_Icall *this_c, Pu_Hdr* pu_hdr_entry, long pu_ofst,
					       FILE *fp, char *fname);

  extern void fb_reader_Info_Edge_list_Read(LIBFB_Info_Edge *this_c, Pu_Hdr* pu_hdr_entry, long pu_ofst,
					    FILE *fp, char *fname);
  extern void fb_reader_Info_Stride_list_Read(LIBFB_Info_Stride *this_c, Pu_Hdr* pu_hdr_entry, long pu_ofst,
					      FILE *fp, char *fname);
  extern void fb_reader_Info_Value_list_Read(LIBFB_Info_Value *this_c, Pu_Hdr* pu_hdr_entry, long pu_ofst,
					       FILE *fp, char *fname);
  extern void fb_reader_Info_Value_FP_Bin_list_Read(LIBFB_Info_Value_FP_Bin *this_c, Pu_Hdr* pu_hdr_entry, long pu_ofst,
						    FILE *fp, char *fname);
  extern void fb_reader_Get_Sizes_Of_Switches(INT32 *this_c, Pu_Hdr* pu_hdr_entry, long pu_ofst,
					      FILE *fp, char *fname);
  extern void fb_reader_Get_Sizes_Of_CGotos(INT32 *this_c, Pu_Hdr* pu_hdr_entry, long pu_ofst,
					    FILE *fp, char *fname);
  extern void fb_reader_Info_Switch_begin(Pu_Hdr pu_hdr_entry, long pu_ofst,
				      FILE *fp, char *fname);

  extern void fb_reader_Info_CGoto_begin(Pu_Hdr pu_hdr_entry, long pu_ofst,
				   FILE *fp, char *fname);

  extern void fb_reader_Str_Table(FILE *fp, char *fname, Fb_Hdr* fb_hdr, char *str_table);
  
  
#ifdef __cplusplus
}
#endif /* __cplusplus */

// ====================================================================
#endif /* le2be_reader.h */
