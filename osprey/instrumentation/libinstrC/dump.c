//-*-c++-*-
// ====================================================================
// ====================================================================
//
// Module: dump.cxx
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
// Write to a binary file all of the frequency counts obtained during
// this run of this program.
//
// ====================================================================
// ====================================================================

#include "myalloc.h"
// [HK] added string.h for mem* prototypes
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "profile_aux.h"
#include "profile_error.h"
#include "libfb_writer.h"
#include "dump.h"
#ifdef INSTR_DEBUG
#include "profile_aux_print.h"
#endif
#define FWRITE(_buffer, _size, _nitems, _fp, _error_message, _error_arg) \
        do { \
	if (fwrite((void *)_buffer, _size, _nitems, _fp) != _nitems) \
	   profile_error(_error_message, _error_arg); \
	} while (0)

#define FSEEK(_fd, _position, whence, _error_message, _error_arg) \
        { \
        if (fseek(_fd, _position, whence) != 0) \
	   profile_error(_error_message, _error_arg); \
	}
extern PROFILE_PHASE Instrumentation_Phase_Num(void);
/* Declaration of static functions */
static void Dump_Fb_File_Header(FILE *fp, char *output_filename,
				Fb_Hdr *fb_hdr);
static void Dump_PU_Profile(FILE *fp, PU_PROFILE_HANDLE pu_handle, char * fname,
			    Pu_Hdr **Pu_Hdr_Table, char **Str_Table);

static void Dump_Fb_File_Pu_Table(FILE *fp, char *fname, Pu_Hdr **Pu_Hdr_Table, size_t Pu_Hdr_Table_Size,
				  Fb_Hdr *fb_hdr);

static void Dump_Fb_File_Str_Table(FILE *fp, char *fname, char **Str_Table, size_t Str_Table_Size,
				   Fb_Hdr *fb_hdr);



// ====================================================================


static char* ERR_POS = "Error in positioning within %s";
static char* ERR_WRITE = "Error in writing to %s";

// round up "offset" as specified by "alignment"
inline ULONG
align (ULONG offset, UINT32 alignment)
{
  --alignment;
  return ((offset + alignment) & ~alignment);
}


inline void
realign_file (FILE* fp, ULONG *offset, UINT32 alignment, char* fname)
{
  ULONG new_offset;
  if (*offset % alignment == 0)
    return;
  
  new_offset = align (*offset, alignment);
  FSEEK (fp, new_offset - *offset, SEEK_CUR, ERR_POS, fname);
  *offset = new_offset;
}

/*
  are we working on a little endian host?
 */
inline ULONG is_host_little_endian()
{
 union
 {
   int l;
   char c[sizeof (int)];
 } u;
 u.l = 1;
 if ( u.c[sizeof (int) - 1] == 1)
   return 1;
//    printf("big endian\n");
 else
   return 0;
//    printf("little endian\n");
}

// Just before finish, dump all the profile information

void 
Dump_all(FILE *fp, char *output_filename)
{
  hash_iterator_t iterator;
//   int i;
  Pu_Hdr **Pu_Hdr_Table = (Pu_Hdr **)alloca(hash_count(PU_Profile_Handle_Table) * sizeof(Pu_Hdr *));
  char **Str_Table = (char **)alloca(hash_count(PU_Profile_Handle_Table) * sizeof(char *));
  Fb_Hdr fb_hdr;
#ifdef INSTR_DEBUG
  fprintf(stderr,"Dump_all for %s\n", output_filename);
#endif
  memset (fb_hdr.fb_ident,0,FB_NIDENT);
  strcpy ((char *) fb_hdr.fb_ident, INSTR_MAG);
  fb_hdr.fb_endianness = is_host_little_endian();
  fb_hdr.fb_version = INSTR_CURRENT;
  fb_hdr.fb_pu_hdr_ent_size = sizeof(Pu_Hdr);
  fb_hdr.phase_num = Instrumentation_Phase_Num();
  fb_hdr.fb_profile_offset = align (sizeof(Fb_Hdr), sizeof(mUINT64));

  // Leave space for Fb_Hdr
  FSEEK(fp, fb_hdr.fb_profile_offset, SEEK_SET, ERR_POS, output_filename);

  // Dump profile info for all PU
  for(iterator = hash_iterator_new(PU_Profile_Handle_Table); 
      !hash_iterator_at_end(iterator); 
      hash_iterator_advance(iterator)) {
    hash_bucket_t bucket = hash_iterator_current(iterator);
    PU_PROFILE_HANDLE *pu_handle = (PU_PROFILE_HANDLE*)hash_bucket_data(bucket);

#ifdef INSTR_DEBUG
  fprintf(stderr,"####################################### \n");
  fprintf(stderr,"Dump_all for %s \n",(*pu_handle)->pu_name );
    PU_Profile_Handle_Print(*pu_handle, stdout);
  fprintf(stderr,"####################################### \n");
#endif
    Dump_PU_Profile(fp, *pu_handle, output_filename, Pu_Hdr_Table, Str_Table);
  }
  iterator = hash_iterator_del(iterator);

  // Now attach the PU header table

  Dump_Fb_File_Pu_Table(fp, output_filename, Pu_Hdr_Table, hash_count(PU_Profile_Handle_Table), &fb_hdr);
//   for (i = 0; i < hash_count(PU_Profile_Handle_Table); i++)
//     MYFREE(Pu_Hdr_Table[i]);
//   MYFREE(Pu_Hdr_Table);
  // Attach the string table 

  Dump_Fb_File_Str_Table(fp, output_filename, Str_Table, hash_count(PU_Profile_Handle_Table), &fb_hdr);
//   MYFREE(Str_Table);

  // Go put the Fb_Hdr at the top of the feedback file
  Dump_Fb_File_Header(fp, output_filename,&fb_hdr);
}

static void
Dump_Fb_File_Header(FILE *fp, char *output_filename, Fb_Hdr *fb_hdr) 
{
  // Rewind the file
  FSEEK(fp,0,SEEK_SET, ERR_POS, output_filename);
  
  // Write the feedback header
  FWRITE(fb_hdr, sizeof(Fb_Hdr), 1, fp, ERR_WRITE, output_filename);

  // Reset the file position to EOF
  FSEEK(fp,0,SEEK_END, ERR_POS, output_filename);
}

static int Type_fbinfo_size_of(FB_Info_Type type)
{
  switch (type) {
  case LIBFB_Info_Invoke_Type:
    return sizeof(LIBFB_Info_Invoke);
  case LIBFB_Info_Branch_Type:
    return sizeof(LIBFB_Info_Branch);
  case LIBFB_Info_Circuit_Type:
    return sizeof(LIBFB_Info_Circuit);
  case LIBFB_Info_Call_Type:
    return sizeof(LIBFB_Info_Call);
  case LIBFB_Info_Loop_Type:
    return sizeof(LIBFB_Info_Loop);
  case LIBFB_Info_Value_Type:
    return sizeof(LIBFB_Info_Value);
  case LIBFB_Info_Value_FP_Bin_Type:
    return sizeof(LIBFB_Info_Value_FP_Bin);
  case LIBFB_Info_Icall_Type:
    return sizeof(LIBFB_Info_Icall);
  default:
    profile_error("Unexpexted Type","");
    return 0;
    break;
  }
}

typedef struct POSITION {
  ULONG offset;
  INT32 num_entries;
} POSITION;

static void POSITION_ULONG_INT32(POSITION *pos, ULONG ofst, INT32 num){
  pos->offset = ofst;
  pos->num_entries = num;
}

static void  fwrite_fb_info_vector(FILE *fp, const void *array, 
				   const char *fname, FB_Info_Type type) {
  FWRITE (&(((LIBFB_Info_Invoke_Vector*)array)->data[0]), Type_fbinfo_size_of(type),
	  ((LIBFB_Info_Invoke_Vector*)array)->size, fp, ERR_WRITE, (char *)fname);
}

static void
Dump_PU_Profile_bis (POSITION *pos, FILE *fp, ULONG *offset, 
		       const void* src, char *fname, 
		       FB_Info_Type type) {

  if (src != NULL) {
    realign_file (fp, offset, sizeof(mINT64), fname);
    
    POSITION_ULONG_INT32(pos, *offset, ((LIBFB_Info_Invoke_Vector *)src)->size);
    
    fwrite_fb_info_vector(fp, src, fname, type);
    
    *offset += ((LIBFB_Info_Invoke_Vector *)src)->size * Type_fbinfo_size_of(type);
  } else {
    POSITION_ULONG_INT32(pos, *offset, 0);
  }
}

// void *profile = *Switch_Profile_Table or *Compgoto_Profile_Table
static void
Dump_PU_Switch_Profile_bis (POSITION *pos, FILE *fp, ULONG *offset, 
			    void *profile, ULONG *target_offset, 
			    char *fname) {

  int i;
  size_t size = profile ? ((Switch_Profile_Vector*)profile)->size : 0;
  realign_file (fp, offset, sizeof(mINT64), fname);
  
  POSITION_ULONG_INT32(pos, *offset, size);
  
  if (profile) {
    i = 0;
    while (i < size) {
      int freq_targets_sz = (((Switch_Profile_Vector*)profile)->data[i]).targets_profile_size;
      INT64 *targets_profile = (((Switch_Profile_Vector*)profile)->data[i]).targets_profile;
      FWRITE (targets_profile, sizeof(INT64),
	      freq_targets_sz, fp, ERR_WRITE, fname);
      *offset += sizeof(INT64) * freq_targets_sz;
      ++i;
    }
  }
  *target_offset = *offset;
  
  if (profile) {
    i = 0;
    while (i < size) {
      INT32 num_targs = (((Switch_Profile_Vector*)profile)->data[i]).targets_profile_size;
      FWRITE (&num_targs, sizeof(INT32), 1, fp, ERR_WRITE, fname);
      ++i;
    }
    *offset += size * sizeof(INT32);
  }
}


// Given a PU handle, dump all the profile information that
// has been gathered for that PU.
static void
Dump_PU_Profile(FILE *fp, PU_PROFILE_HANDLE pu_handle, char * fname,
		Pu_Hdr **Pu_Hdr_Table,  char **Str_Table)
{
  static ULONG Str_Offset = 0;
  static ULONG PU_Offset = 0;
  static int Pu_Hdr_Table_Index = 0;
  Pu_Hdr *pu_hdr = (Pu_Hdr *)MYMALLOC(sizeof(Pu_Hdr));
  ULONG offset = 0;
  POSITION pos;

  // Update the string Table
  Str_Table[Pu_Hdr_Table_Index] = pu_handle->pu_name;
  // double word align the PU_Offset
  realign_file (fp, &PU_Offset, sizeof(mINT64), fname);
  
  pu_hdr->pu_checksum = pu_handle->checksum;
  pu_hdr->pu_size = pu_handle->pu_size;
  pu_hdr->runtime_fun_address = pu_handle->runtime_fun_address;
  pu_hdr->pu_name_index = Str_Offset;
  pu_hdr->pu_file_offset = PU_Offset;

  {
    Dump_PU_Profile_bis (&pos, fp, &offset, pu_handle->LIBFB_Info_Invoke_Table, fname, LIBFB_Info_Invoke_Type);
    pu_hdr->pu_inv_offset = pos.offset;
    pu_hdr->pu_num_inv_entries = pos.num_entries;
#ifdef INSTR_DEBUG
    /* Dump fb_info */
    if (pu_handle->LIBFB_Info_Invoke_Table)
    {
      size_t size, i;
      fprintf( stdout, "fb_info invoke = " );
      size = pu_handle->LIBFB_Info_Invoke_Table->size;
      for (i = 0; i < size; i++) {
	fprintf(stdout, "\tid = %d\t", i);
	LIBFB_Info_Invoke_Print(&(pu_handle->LIBFB_Info_Invoke_Table->data[i]), stdout);
      }
      fprintf(stdout, "\n");
    }
#endif
  }

  {
    Dump_PU_Profile_bis (&pos, fp, &offset, pu_handle->LIBFB_Info_Branch_Table, fname, LIBFB_Info_Branch_Type);
    pu_hdr->pu_br_offset = pos.offset;
    pu_hdr->pu_num_br_entries = pos.num_entries;
#ifdef INSTR_DEBUG
    /* Dump fb_info */
    if (pu_handle->LIBFB_Info_Branch_Table)
    {
      size_t size, i;
      fprintf( stdout, "fb_info branch = " );
      size = pu_handle->LIBFB_Info_Branch_Table->size;
      for (i = 0; i < size; i++) {
	fprintf(stdout, "\tid = %d\t", i);
	LIBFB_Info_Branch_Print(&(pu_handle->LIBFB_Info_Branch_Table->data[i]), stdout);
      }
      fprintf(stdout, "\n");
    }
#endif
  }

  {
    Dump_PU_Switch_Profile_bis (&pos, fp, &offset, pu_handle->Switch_Profile_Table,
				      &(pu_hdr->pu_switch_target_offset), fname);
    pu_hdr->pu_switch_offset = pos.offset;
    pu_hdr->pu_num_switch_entries = pos.num_entries;
  }

  {
    Dump_PU_Switch_Profile_bis (&pos, fp, &offset, pu_handle->Compgoto_Profile_Table,
				      &(pu_hdr->pu_cgoto_target_offset), fname);
    pu_hdr->pu_cgoto_offset = pos.offset;
    pu_hdr->pu_num_cgoto_entries = pos.num_entries;
  }

  {
    Dump_PU_Profile_bis (&pos, fp, &offset, pu_handle->LIBFB_Info_Loop_Table, fname, LIBFB_Info_Loop_Type);
    pu_hdr->pu_loop_offset = pos.offset;
    pu_hdr->pu_num_loop_entries = pos.num_entries;
#ifdef INSTR_DEBUG
    /* Dump fb_info */
    if (pu_handle->LIBFB_Info_Loop_Table)
    {
      size_t size, i;
      fprintf( stdout, "fb_info loop = " );
      size = pu_handle->LIBFB_Info_Loop_Table->size;
      for (i = 0; i < size; i++) {
	fprintf(stdout, "\tid = %d\t", i);
	LIBFB_Info_Loop_Print(&(pu_handle->LIBFB_Info_Loop_Table->data[i]), stdout);
      }
      fprintf(stdout, "\n");
    }
#endif
  }

  {
    Dump_PU_Profile_bis (&pos, fp, &offset, pu_handle->LIBFB_Info_Circuit_Table, fname, LIBFB_Info_Circuit_Type); 
    pu_hdr->pu_scircuit_offset = pos.offset;
    pu_hdr->pu_num_scircuit_entries = pos.num_entries;
#ifdef INSTR_DEBUG
    /* Dump fb_info */
    if (pu_handle->LIBFB_Info_Circuit_Table)
    {
      size_t size, i;
      fprintf( stdout, "fb_info circuit = " );
      size = pu_handle->LIBFB_Info_Circuit_Table->size;
      for (i = 0; i < size; i++) {
	fprintf(stdout, "\tid = %d\t", i);
	LIBFB_Info_Circuit_Print(&(pu_handle->LIBFB_Info_Circuit_Table->data[i]), stdout);
      }
      fprintf(stdout, "\n");
    }
#endif
  }

  {
    Dump_PU_Profile_bis (&pos, fp, &offset, pu_handle->LIBFB_Info_Call_Table,
			   fname, LIBFB_Info_Call_Type);
    pu_hdr->pu_call_offset = pos.offset;
    pu_hdr->pu_num_call_entries = pos.num_entries;
#ifdef INSTR_DEBUG
    /* Dump fb_info */
    if (pu_handle->LIBFB_Info_Call_Table)
    {
      size_t i;
      fprintf( stdout, "fb_info call = " );
      for (i = 0; i < pu_handle->LIBFB_Info_Call_Table->size; i++) {
	fprintf(stdout, "\tid = %d\t", i);
	LIBFB_Info_Call_Print(&(pu_handle->LIBFB_Info_Call_Table->data[i]), stdout);
      }
      fprintf(stdout, "\n");
    }
#endif
  }

  {
    Dump_PU_Profile_bis (&pos, fp, &offset, pu_handle->LIBFB_Info_Value_Table,
			   fname, LIBFB_Info_Value_Type);
    pu_hdr->pu_value_offset = pos.offset;
    pu_hdr->pu_num_value_entries = pos.num_entries;
#ifdef INSTR_DEBUG
    /* Dump fb_info */
    if (pu_handle->LIBFB_Info_Value_Table)
    {
      size_t i;
      fprintf( stdout, "fb_info value = " );
      for (i = 0; i < pu_handle->LIBFB_Info_Value_Table->size; i++) {
	fprintf(stdout, "\tid = %d\t", i);
	LIBFB_Info_Value_Print(&(pu_handle->LIBFB_Info_Value_Table->data[i]), stdout);
      }
      fprintf(stdout, "\n");
    }
#endif
  }

  {
    Dump_PU_Profile_bis (&pos, fp, &offset, pu_handle->LIBFB_Info_Value_Table,
			   fname, LIBFB_Info_Value_Type);
    pu_hdr->pu_value_offset = pos.offset;
    pu_hdr->pu_num_value_entries = pos.num_entries;
#ifdef INSTR_DEBUG
    /* Dump fb_info */
    if (pu_handle->LIBFB_Info_Value_Table)
    {
      size_t i;
      fprintf( stdout, "fb_info value = " );
      for (i = 0; i < pu_handle->LIBFB_Info_Value_Table->size; i++) {
	fprintf(stdout, "\tid = %d\t", i);
	LIBFB_Info_Value_Print(&(pu_handle->LIBFB_Info_Value_Table->data[i]), stdout);
      }
      fprintf(stdout, "\n");
    }
#endif
  }

  {
    Dump_PU_Profile_bis (&pos, fp, &offset, pu_handle->LIBFB_Info_Value_Table,
			   fname, LIBFB_Info_Value_Type);
    pu_hdr->pu_value_offset = pos.offset;
    pu_hdr->pu_num_value_entries = pos.num_entries;
#ifdef INSTR_DEBUG
    /* Dump fb_info */
    if (pu_handle->LIBFB_Info_Value_Table)
    {
      size_t i;
      fprintf( stdout, "fb_info value = " );
      for (i = 0; i < pu_handle->LIBFB_Info_Value_Table->size; i++) {
	fprintf(stdout, "\tid = %d\t", i);
	LIBFB_Info_Value_Print(&(pu_handle->LIBFB_Info_Value_Table->data[i]), stdout);
      }
      fprintf(stdout, "\n");
    }
#endif
  }

  {
    Dump_PU_Profile_bis (&pos, fp, &offset, pu_handle->LIBFB_Info_Value_FP_Bin_Table,
			   fname, LIBFB_Info_Value_FP_Bin_Type);
    pu_hdr->pu_value_fp_bin_offset = pos.offset;
    pu_hdr->pu_num_value_fp_bin_entries = pos.num_entries;
#ifdef INSTR_DEBUG
    /* Dump fb_info */
    if (pu_handle->LIBFB_Info_Value_FP_Bin_Table)
    {
      size_t i;
      fprintf( stdout, "fb_info value_fp_bin = " );
      for (i = 0; i < pu_handle->LIBFB_Info_Value_FP_Bin_Table->size; i++) {
	fprintf(stdout, "\tid = %d\t", i);
	LIBFB_Info_Value_FP_Bin_Print(&(pu_handle->LIBFB_Info_Value_FP_Bin_Table->data[i]), stdout);
      }
      fprintf(stdout, "\n");
    }
#endif
  }

  {
    Dump_PU_Profile_bis (&pos, fp, &offset, pu_handle->LIBFB_Info_Icall_Table,
			   fname, LIBFB_Info_Icall_Type);
    pu_hdr->pu_icall_offset = pos.offset;
    pu_hdr->pu_num_icall_entries = pos.num_entries;
#ifdef INSTR_DEBUG
    /* Dump fb_info */
    if (pu_handle->LIBFB_Info_Icall_Table)
    {
      size_t i;
      fprintf( stdout, "fb_info icall = " );
      for (i = 0; i < pu_handle->LIBFB_Info_Icall_Table->size; i++) {
	fprintf(stdout, "\tid = %d\t", i);
	LIBFB_Info_Icall_Print(&(pu_handle->LIBFB_Info_Icall_Table->data[i]), stdout);
      }
      fprintf(stdout, "\n");
    }
#endif
  }

  // Enter the PU header
#ifdef INSTR_DEBUG
  Pu_Hdr_Print(pu_hdr, stdout);
#endif
  Pu_Hdr_Table[Pu_Hdr_Table_Index] = pu_hdr;
  Pu_Hdr_Table_Index++;
  Str_Offset += strlen(pu_handle->pu_name) + 1;
  PU_Offset += (offset - pu_hdr->pu_inv_offset);
}



// Write out the PU header table

static void 
Dump_Fb_File_Pu_Table(FILE *fp, char *fname, Pu_Hdr **Pu_Hdr_Table, size_t Pu_Hdr_Table_Size,
		      Fb_Hdr *fb_hdr)
{
  int i;
  ULONG offset = ftell(fp);
  realign_file (fp, &offset, 
#if defined(__GNUC__)
		//		__alignof__(Pu_Hdr),
		4,
#else
		__builtin_alignof(Pu_Hdr), 
#endif
		fname);

  fb_hdr->fb_pu_hdr_offset = offset;
  fb_hdr->fb_pu_hdr_num = Pu_Hdr_Table_Size;
 
  for (i = 0; i < Pu_Hdr_Table_Size; i++) {
    Pu_Hdr *pu_hdr_entry = Pu_Hdr_Table[i];
    FWRITE(pu_hdr_entry, sizeof(Pu_Hdr), 1, fp, ERR_WRITE, fname);
  }
}


// Write out the string table

static void 
Dump_Fb_File_Str_Table(FILE *fp, char *fname, char **Str_Table, size_t Str_Table_Size,
		       Fb_Hdr *fb_hdr)
{
  ULONG table_size = 0;
  int i;
  fb_hdr->fb_str_table_offset = ftell(fp);

  for (i = 0; i < Str_Table_Size; i++) {
    char *pu_name = Str_Table[i];
    FWRITE(pu_name, strlen(pu_name) + 1, 1, fp, ERR_WRITE, fname);
    table_size += strlen(pu_name) + 1;
  }

  fb_hdr->fb_str_table_size = table_size;
}
