// ====================================================================
//
// Description:
//
// Propose conversion writer from big to little 
// 
// 
//
// ====================================================================
// ====================================================================


#ifndef libfb_writer_INCLUDED
#define libfb_writer_INCLUDED
#include <stdio.h>
#include "libfb_info.h"

typedef enum FB_Info_Type {
  LIBFB_Info_Invoke_Type,
  LIBFB_Info_Branch_Type,
  LIBFB_Info_Circuit_Type,
  LIBFB_Info_Call_Type,
  LIBFB_Info_Loop_Type
} FB_Info_Type;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

extern void FB_Info_Write(const void *this_c, FB_Info_Type type, FILE *fp, char *output_filename);
extern void Fb_Hdr_Write(const Fb_Hdr *this_c, FILE *fp, char *output_filename);
extern void Pu_Hdr_Write(const Pu_Hdr *this_c, FILE *fp, char *output_filename);
extern void INT64_list_Write(const INT64 *this_c, int nitems, FILE *fp, char *output_filename);
extern void INT32_Write(const INT32 this_c, FILE *fp, char *output_filename);

#ifdef __cplusplus
}
#endif /* __cplusplus */

// ====================================================================
#endif /* libfb_writer_INCLUDED */
