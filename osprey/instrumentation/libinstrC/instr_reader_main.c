
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libfb_reader.h"
#include "libfb_info_print.h"
#include "profile_error.h"
#include "myalloc.h"

typedef struct Fb_File_Info {
    char *name;
    FILE *fp;
    Fb_Hdr *fb_hdr;
    Pu_Hdr *pu_hdr_table;
    char *str_table;
} Fb_File_Info;

Fb_File_Info file_info;

void
Process_Feedback_File(char *fb_name)
{
  FILE *fp;
  Fb_Hdr *fb_hdr;
  Pu_Hdr *pu_hdr_table;
  char *str_table;
  PROFILE_PHASE phase_num;

  if ((fp = fopen(fb_name, "r")) == NULL) {
       profile_error("Unable to open file: %s", fb_name);
       exit(1);
  }

  fb_hdr = (Fb_Hdr *)MYMALLOC(sizeof(Fb_Hdr));
  fb_reader_Fb_Hdr_Read(fb_hdr, fp, fb_name);

  pu_hdr_table = (Pu_Hdr *) MYMALLOC(sizeof(Pu_Hdr)*fb_hdr->fb_pu_hdr_num);

  fb_reader_Pu_Hdr_list_Read(pu_hdr_table, fb_hdr, fp, fb_name);

  str_table = (char *)MYMALLOC(sizeof(char) * fb_hdr->fb_str_table_size);
  fb_reader_Str_Table(fp, fb_name, fb_hdr, str_table);

  phase_num = fb_hdr->phase_num;

  file_info.name = strdup(fb_name);
  file_info.fp = fp;
  file_info.fb_hdr = fb_hdr;
  file_info.pu_hdr_table = pu_hdr_table;
  file_info.str_table = str_table;
  
}

int main(int argc, char **argv)
{
  char *filein = NULL;
  char output_in_files = 0;
  int i;
  for (i = 1; i < argc; i++) {
    if (argv[i][0] == '-') {
      if (strcmp(&argv[i][1], "files") == 0)
	  output_in_files = 1;
    }
    else if (filein == NULL)
      filein = argv[i];
    else {
      fprintf (stderr, "Too many arguments\n");
      return 1;
    }
  }
  if (filein == NULL) {
    fprintf (stderr, "No specified file\n");
    return 1;
  }
  Process_Feedback_File(filein);

  {
    FILE *fp= file_info.fp; 
    char *fb_fname= file_info.name; 
    Fb_Hdr *fb_hdr= file_info.fb_hdr;
    Pu_Hdr *pu_hdr_table= file_info.pu_hdr_table; 
    char *str_table = file_info.str_table; 
    Pu_Hdr pu_hdr_entry;
    long pu_ofst;
    char *entry_name;
    unsigned long i;
    for (i = 0; i < fb_hdr->fb_pu_hdr_num; i++) {
      int j;
      FILE *ofile;
      char *output_file;
      pu_hdr_entry = pu_hdr_table[i];
      entry_name = str_table + pu_hdr_entry.pu_name_index;
      
      pu_ofst = fb_hdr->fb_profile_offset + pu_hdr_entry.pu_file_offset;
      
      if (output_in_files) {
	output_file = strdup(entry_name);
	for (j = 0; j < strlen(output_file); j++)
	  if (output_file[j] == '/') output_file[j] = '_';
	ofile = fopen(output_file, "w");
      } else ofile = stdout;

      fprintf( ofile, "\nFeedback data of %s (checksum %d)\n", entry_name, pu_hdr_entry.pu_checksum);
      {
	LIBFB_Info_Invoke *fb_info;
	size_t size = pu_hdr_entry.pu_num_inv_entries, i;
	fb_info = (LIBFB_Info_Invoke *)MYMALLOC(sizeof(LIBFB_Info_Invoke) * size);;


	fb_reader_Info_Invoke_list_Read(fb_info, &pu_hdr_entry, pu_ofst, fp, fb_fname);

	/* Dump fb_info */
	if (size)
	  fprintf( ofile, "invoke = " );
	for (i = 0; i < size; i++) {
	  fprintf(ofile, "\n\tid = %d\t", i);
	  LIBFB_Info_Invoke_Print(&(fb_info[i]), ofile);
	}
	MYFREE (fb_info);
	if (size)
	  fprintf(ofile, "\n");
      }

      {
	LIBFB_Info_Branch *fb_info;
	size_t size = pu_hdr_entry.pu_num_br_entries, i;
	fb_info = (LIBFB_Info_Branch *)MYMALLOC(sizeof(LIBFB_Info_Branch) * size);;


	fb_reader_Info_Branch_list_Read(fb_info, &pu_hdr_entry, pu_ofst, fp, fb_fname);

	/* Dump fb_info */
	if (size)
	  fprintf( ofile, "branch = " );
	for (i = 0; i < size; i++) {
	  fprintf(ofile, "\n\tid = %d\t", i);
	  LIBFB_Info_Branch_Print(&(fb_info[i]), ofile);
	}
	MYFREE (fb_info);
	if (size)
	  fprintf(ofile, "\n");
      }


      {
	LIBFB_Info_Loop *fb_info;
	size_t size = pu_hdr_entry.pu_num_loop_entries, i;
	fb_info = (LIBFB_Info_Loop *)MYMALLOC(sizeof(LIBFB_Info_Loop) * size);;


	fb_reader_Info_Loop_list_Read(fb_info, &pu_hdr_entry, pu_ofst, fp, fb_fname);

	/* Dump fb_info */
	if (size)
	  fprintf( ofile, "loop = " );
	for (i = 0; i < size; i++) {
	  fprintf(ofile, "\n\tid = %d\t", i);
	  LIBFB_Info_Loop_Print(&(fb_info[i]), ofile);
	}
	MYFREE (fb_info);
	if (size)
	  fprintf(ofile, "\n");
      }

      {
	LIBFB_Info_Circuit *fb_info;
	size_t size = pu_hdr_entry.pu_num_scircuit_entries, i;
	fb_info = (LIBFB_Info_Circuit *)MYMALLOC(sizeof(LIBFB_Info_Circuit) * size);;


	fb_reader_Info_Circuit_list_Read(fb_info, &pu_hdr_entry, pu_ofst, fp, fb_fname);

	/* Dump fb_info */
	if (size)
	  fprintf( ofile, "Short Circuit = " );
	for (i = 0; i < size; i++) {
	  fprintf(ofile, "\n\tid = %d\t", i);
	  LIBFB_Info_Circuit_Print(&(fb_info[i]), ofile);
	}
	MYFREE (fb_info);
	if (size)
	  fprintf(ofile, "\n");
      }

      {
	LIBFB_Info_Call *fb_info;
	size_t size = pu_hdr_entry.pu_num_call_entries, i;
	fb_info = (LIBFB_Info_Call *)MYMALLOC(sizeof(LIBFB_Info_Call) * size);;

	fb_reader_Info_Call_list_Read(fb_info, &pu_hdr_entry, pu_ofst, fp, fb_fname);

	/* Dump fb_info */
	if (size)
	  fprintf( ofile, "Call = ");
	for (i = 0; i < size; i++) {
	  fprintf(ofile, "\n\tid = %d\t", i);
	  LIBFB_Info_Call_Print(&(fb_info[i]), ofile);
	}
	MYFREE (fb_info);
	if (size)
	  fprintf(ofile, "\n");
      }

      {
	INT32 *target;
	size_t size = pu_hdr_entry.pu_num_switch_entries, i;
	target = (INT32 *)MYMALLOC(sizeof(INT32) * size);;
	
	// Get sizes of switches
	fb_reader_Get_Sizes_Of_Switches(target, &pu_hdr_entry, pu_ofst, fp, fb_fname);
	// Set the right position in file
	fb_reader_Info_Switch_begin(pu_hdr_entry, pu_ofst, fp, fb_fname);

	if (size)
	  fprintf( ofile, "Switch = ");
	/* Dump fb_info */
	for (i = 0; i < size; i++) {
	  int t;
	  INT64 *freq_targets = (INT64 *)MYMALLOC(target[i] * sizeof(INT64)); 
	  fprintf( ofile, "\n\tid = %d\t", i);
	  fprintf( ofile, "targets = %d", target[i]);
	  fb_reader_INT64_list_Read(freq_targets, target[i], fp, fb_fname);

	  for (t = 0; t < target[i]; t++ ) {
	    fprintf( ofile, ", %d: ", t );
	    INT64_Print( freq_targets[t], ofile );
	  }
	  MYFREE(freq_targets);
	}
	MYFREE (target);
	if (size)
	  fprintf(ofile, "\n");
      }

      {
	INT32 *target;
	size_t size = pu_hdr_entry.pu_num_cgoto_entries, i;
	target = (INT32 *)MYMALLOC(sizeof(INT32) * size);;
	
	fb_reader_Get_Sizes_Of_CGotos(target, &pu_hdr_entry, pu_ofst, fp, fb_fname);
	// Set the right position in file
	fb_reader_Info_CGoto_begin(pu_hdr_entry, pu_ofst, fp, fb_fname);

	if (size)
	  fprintf( ofile, "Computed goto = ");
	/* Dump fb_info */
	for (i = 0; i < size; i++) {
	  int t;
	  INT64 *freq_targets = (INT64 *)MYMALLOC(target[i] * sizeof(INT64)); 
	  fprintf( ofile, "Conputeed goto nb %d = ", i);
	  fprintf( ofile, "FB---> targets = %d", target[i]);
	  fb_reader_INT64_list_Read(freq_targets, target[i], fp, fb_fname);

	  for (t = 0; t < target[i]; t++ ) {
	    fprintf( ofile, ", %d: ", t );
	    INT64_Print( freq_targets[t], ofile );
	  }
	  MYFREE(freq_targets);
	}
	MYFREE (target);
	if (size)
	  fprintf(ofile, "\n");
      }

      if (output_in_files)
	fclose(ofile);
    }
  }
  MYFREE(file_info.name);
  MYFREE(file_info.fb_hdr);
  MYFREE(file_info.pu_hdr_table);
  MYFREE(file_info.str_table);
  return 0;
}
