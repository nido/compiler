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

#include "libfb_reader.h"
#include "profile_error.h"

#include <assert.h>
#ifndef ASSERT
#ifdef DEBUG
#define ASSERT(a) do {assert(a);} while(0);
#else
#define ASSERT(a) 
/* do {;} while(0) */
#endif
#endif


static char* ERR_POS = "Error in positioning within %s";
static char* ERR_READ = "Error in reading from %s";
static BOOL unneed_conversion = 0;
#define FREAD(_buffer, _size, _nitems, _fp, _error_message, _error_arg) \
        if (fread((void *)_buffer, _size, _nitems, _fp) != _nitems) \
           profile_error(_error_message, _error_arg);


#define FSEEK(_fd, _position, whence, _error_message, _error_arg) \
        { \
        if (fseek(_fd, _position, whence) != 0) \
	   profile_error(_error_message, _error_arg); \
	}

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */



  /*****************************************************************

                    READER FUNCTIONS

  *****************************************************************/
  typedef enum FB_Info_Type {
    LIBFB_Info_Invoke_Type,
    LIBFB_Info_Branch_Type,
    LIBFB_Info_Circuit_Type,
    LIBFB_Info_Call_Type,
    LIBFB_Info_Loop_Type
  } FB_Info_Type;



  inline void int_Read(int *this_c, FILE *fp, char *fname){
    char ptr[4];
    union u {
      int f;
      char b[4];
    } p;
    ASSERT(!unneed_conversion);

    FREAD(ptr, sizeof(char), 4, fp, ERR_READ, fname);

    p.b[3] = ptr[0];
    p.b[2] = ptr[1];
    p.b[1] = ptr[2];
    p.b[0] = ptr[3];

    *this_c = p.f;
  }

  inline void float_Read(float *this_c, FILE *fp, char *fname){
    char ptr[4];
    union u {
      float f;
      char b[4];
    } p;
    ASSERT(!unneed_conversion);
    FREAD(ptr, sizeof(char), 4, fp, ERR_READ, fname);

    p.b[3] = ptr[0];
    p.b[2] = ptr[1];
    p.b[1] = ptr[2];
    p.b[0] = ptr[3];

    *this_c = p.f;
  }

#define INT32_Read int_Read
#if 0
  static void INT32_Read(INT32 *this_c, FILE *fp, char *fname){
    int_Read(this_c, fp, fname);
  }
#endif

  static void ULONG_Read(ULONG *this_c, FILE *fp, char *fname)
  {
    int_Read(this_c, fp, fname);
  }

/*   static void BOOL_Read(BOOL *this_c, FILE *fp, char *fname) { */
/*     int_Read(this_c, fp, fname); */
/*   } */

  static void enum_Read(int *this_c, FILE *fp, char *fname) {
    int_Read(this_c, fp, fname);
  }

  static void INT64_Read(INT64 *this_c, FILE *fp, char *fname){
    union u {
      INT64 ll;
      struct dint32 { 
	INT32 l; 
	INT32 h;} dint; 
    } p;
    INT32 first_int, sec_int;
    ASSERT(!unneed_conversion);
    int_Read(&first_int, fp, fname);
    int_Read(&sec_int, fp, fname);
    p.dint.h = first_int;
    p.dint.l = sec_int;
    
    *this_c = p.ll;
  }

  void fb_reader_INT64_list_Read(INT64 *this_c, int nitems, FILE *fp, char *fname){
    int i;

    if (unneed_conversion) {
      FREAD (this_c, sizeof(INT64), nitems, fp,
	     ERR_READ, fname);  
    } else {
      for (i = 0; i < nitems; i++) {
	INT64_Read(&(this_c[i]), fp, fname);
      }
    }
  }
  static void LIBFB_Info_Invoke_Read(LIBFB_Info_Invoke *this_c, FILE *fp, char *fname) {
      INT64_Read(&(this_c->freq_invoke), fp, fname);
    }

    static void LIBFB_Info_Branch_Read( LIBFB_Info_Branch *this_c, FILE *fp, char *fname) {
      INT64_Read( &(this_c->freq_taken), fp, fname);
      INT64_Read( &(this_c->freq_not_taken), fp, fname);
    }

    static void LIBFB_Info_Loop_Read( LIBFB_Info_Loop *this_c, FILE *fp, char *fname) {
      INT64_Read( &(this_c->invocation_count), fp, fname);
      INT64_Read( &(this_c->total_trip_count), fp, fname);
      INT64_Read( &(this_c->last_trip_count), fp, fname);
      INT64_Read( &(this_c->min_trip_count), fp, fname);
      INT64_Read( &(this_c->max_trip_count), fp, fname);
      INT64_Read( &(this_c->num_zero_trips), fp, fname);
    }

    static void LIBFB_Info_Circuit_Read( LIBFB_Info_Circuit *this_c, FILE *fp, char *fname) {
      INT64_Read( &(this_c->freq_left), fp, fname);
      INT64_Read( &(this_c->freq_right), fp, fname);
      INT64_Read( &(this_c->freq_neither), fp, fname);
    }

    static void LIBFB_Info_Call_Read( LIBFB_Info_Call *this_c, FILE *fp, char *fname) {
      INT64_Read( &(this_c->freq_entry), fp, fname);
      INT64_Read( &(this_c->freq_exit), fp, fname);
    }

    static BOOL is_host_little_endian()
      {
	union
	{
	  int l;
	  char c[sizeof (int)];
	} u;
	u.l = 1;
	if ( u.c[sizeof (int) - 1] == 1)
	  return 0;
	//    printf("big endian\n");
	else
	  return 1;
	//    printf("little endian\n");
      }

    // Read the main header in the feedback file
    void fb_reader_Fb_Hdr_Read(Fb_Hdr *this_c, FILE *fp, char *fname){

      FSEEK(fp,0,SEEK_SET,ERR_POS,fname);
      FREAD(&(this_c->fb_ident), FB_NIDENT, 1, fp, ERR_READ, fname);
      FREAD(&(this_c->fb_endianness), 1, sizeof(ULONG), fp, ERR_READ, fname);
#ifdef INSTR_DEBUG
      if (this_c->fb_endianness == 0) {
	printf("Instrumentation file is little endian\n");
      } else {
	printf("Instrumentation file is big endian\n");
      }
      if (is_host_little_endian()) {
	printf("host is little endian\n");
      } else {
	printf("host file is big endian\n");
      }
#endif
      unneed_conversion = ((is_host_little_endian() && (this_c->fb_endianness == 0))
			 || (!is_host_little_endian() && (this_c->fb_endianness != 0)));
#ifdef INSTR_DEBUG
      if (!unneed_conversion)
	printf ("NEED CONVERSION\n");
#endif
      if (unneed_conversion) {
	FSEEK(fp,0,SEEK_SET,ERR_POS,fname);
	FREAD(this_c, sizeof(Fb_Hdr), 1, fp,
	      "Error while writing to: %s", fname);
      } else {
    
	FSEEK(fp,0,SEEK_SET,ERR_POS,fname);
	FREAD(&(this_c->fb_ident), FB_NIDENT, 1, fp, ERR_READ, fname);
	ULONG_Read( &(this_c->fb_endianness), fp, fname);
	ULONG_Read( &(this_c->fb_version), fp, fname);
	ULONG_Read( &(this_c->fb_profile_offset), fp, fname);
	ULONG_Read( &(this_c->fb_pu_hdr_offset), fp, fname); 
	ULONG_Read( &(this_c->fb_pu_hdr_ent_size), fp, fname);
	ULONG_Read( &(this_c->fb_pu_hdr_num), fp, fname);     
	ULONG_Read( &(this_c->fb_str_table_offset), fp, fname);
	ULONG_Read( &(this_c->fb_str_table_size), fp, fname);
	enum_Read( &(this_c->phase_num), fp, fname);
      }  
    }

    void fb_reader_Pu_Hdr_list_Read(Pu_Hdr *this_c, Fb_Hdr *fb_hdr,
				    FILE *fp, char *fname){
      int i;
      int nitems = fb_hdr->fb_pu_hdr_num;

      FSEEK(fp,fb_hdr->fb_pu_hdr_offset,SEEK_SET, ERR_POS, fname);

      if (unneed_conversion) {
	FREAD(this_c, sizeof(Pu_Hdr), nitems, fp, ERR_READ,
	      fname);
      } else {
	for (i = 0; i < nitems; i++) {
	  INT32_Read( &(this_c[i].pu_checksum), fp, fname);
	  ULONG_Read( &(this_c[i].pu_name_index), fp, fname);
	  ULONG_Read( &(this_c[i].pu_file_offset), fp, fname);
	  ULONG_Read( &(this_c[i].pu_inv_offset), fp, fname);
	  ULONG_Read( &(this_c[i].pu_num_inv_entries), fp, fname);
	  ULONG_Read( &(this_c[i].pu_br_offset), fp, fname);
	  ULONG_Read( &(this_c[i].pu_num_br_entries), fp, fname);
	  ULONG_Read( &(this_c[i].pu_switch_offset), fp, fname);
	  ULONG_Read( &(this_c[i].pu_switch_target_offset), fp, fname);
	  ULONG_Read( &(this_c[i].pu_num_switch_entries), fp, fname);
	  ULONG_Read( &(this_c[i].pu_cgoto_offset), fp, fname);
	  ULONG_Read( &(this_c[i].pu_cgoto_target_offset), fp, fname);
	  ULONG_Read( &(this_c[i].pu_num_cgoto_entries), fp, fname);
	  ULONG_Read( &(this_c[i].pu_loop_offset), fp, fname);
	  ULONG_Read( &(this_c[i].pu_num_loop_entries), fp, fname);
	  ULONG_Read( &(this_c[i].pu_scircuit_offset), fp, fname);
	  ULONG_Read( &(this_c[i].pu_num_scircuit_entries), fp, fname);
	  ULONG_Read( &(this_c[i].pu_call_offset), fp, fname);
	  ULONG_Read( &(this_c[i].pu_num_call_entries), fp, fname);
	}
      }  
    }

    static void *FB_Info_Type_Read(FB_Info_Type type)
      {
	switch (type) {
	case LIBFB_Info_Invoke_Type:
	  return (void *)&LIBFB_Info_Invoke_Read;
	case LIBFB_Info_Branch_Type:
	  return (void *)&LIBFB_Info_Branch_Read;
	case LIBFB_Info_Circuit_Type:
	  return (void *)&LIBFB_Info_Circuit_Read;
	case LIBFB_Info_Call_Type:
	  return (void *)&LIBFB_Info_Call_Read;
	case LIBFB_Info_Loop_Type:
	  return (void *)&LIBFB_Info_Loop_Read;
	default:
	  profile_error("Unexpexted Type","");
	  return 0;
	  break;
	}
      }

    typedef  void (*dump_func_type)(void *, FILE*, char*);

    static void fb_reader_Info_Read(void *this_c, FB_Info_Type type, FILE *fp, char *fname)
      {
	dump_func_type dump_func = FB_Info_Type_Read(type); 

	dump_func((void *)this_c, fp, fname);
      }

    void fb_reader_Info_Invoke_list_Read(LIBFB_Info_Invoke *this_c, Pu_Hdr* pu_hdr_entry, long pu_ofst,
					    FILE *fp, char *fname)
      {
	int i;
	FSEEK(fp, pu_ofst + pu_hdr_entry->pu_inv_offset, SEEK_SET, ERR_POS, fname);
	if (unneed_conversion) {
	  FREAD (this_c, sizeof(LIBFB_Info_Invoke),
		 pu_hdr_entry->pu_num_inv_entries, fp, ERR_READ, fname);

	} else {
	  for (i = 0; i < pu_hdr_entry->pu_num_inv_entries; i++) {
	    fb_reader_Info_Read(&(this_c[i]), LIBFB_Info_Invoke_Type, fp, (char *)fname);
	  }
	}  
      }

    void fb_reader_Info_Branch_list_Read(LIBFB_Info_Branch *this_c, Pu_Hdr* pu_hdr_entry, long pu_ofst,
					    FILE *fp, char *fname)
      {
	int i;
	FSEEK(fp, pu_ofst + pu_hdr_entry->pu_br_offset, SEEK_SET, ERR_POS, fname);
	if (unneed_conversion) {
	  FREAD (this_c, sizeof(LIBFB_Info_Branch),
		 pu_hdr_entry->pu_num_br_entries, fp, ERR_READ, fname);

	} else {
	  for (i = 0; i < pu_hdr_entry->pu_num_br_entries; i++) {
	    fb_reader_Info_Read(&(this_c[i]), LIBFB_Info_Branch_Type, fp, (char *)fname);
	  }
	}  
      }

    void fb_reader_Info_Loop_list_Read(LIBFB_Info_Loop *this_c, Pu_Hdr* pu_hdr_entry, long pu_ofst,
					  FILE *fp, char *fname)
      {
	int i;
	FSEEK(fp, pu_ofst + pu_hdr_entry->pu_loop_offset, SEEK_SET, ERR_POS, fname);
	if (unneed_conversion) {
	  FREAD (this_c, sizeof(LIBFB_Info_Loop),
		 pu_hdr_entry->pu_num_loop_entries, fp, ERR_READ, fname);

	} else {
	  for (i = 0; i < pu_hdr_entry->pu_num_loop_entries; i++) {
	    fb_reader_Info_Read(&(this_c[i]), LIBFB_Info_Loop_Type, fp, (char *)fname);
	  }
	}  
      }

    void fb_reader_Info_Circuit_list_Read(LIBFB_Info_Circuit *this_c, Pu_Hdr* pu_hdr_entry, long pu_ofst,
					     FILE *fp, char *fname)
      {
	int i;
	FSEEK(fp, pu_ofst + pu_hdr_entry->pu_scircuit_offset, SEEK_SET, ERR_POS, fname);
	if (unneed_conversion) {
	  FREAD (this_c, sizeof(LIBFB_Info_Circuit),
		 pu_hdr_entry->pu_num_scircuit_entries, fp, ERR_READ, fname);

	} else {
	  for (i = 0; i < pu_hdr_entry->pu_num_scircuit_entries; i++) {
	    fb_reader_Info_Read(&(this_c[i]), LIBFB_Info_Circuit_Type, fp, (char *)fname);
	  }
	}  
      }

    void fb_reader_Info_Call_list_Read(LIBFB_Info_Call *this_c, Pu_Hdr* pu_hdr_entry, long pu_ofst,
					  FILE *fp, char *fname)
      {
	int i;
	FSEEK(fp, pu_ofst + pu_hdr_entry->pu_call_offset, SEEK_SET, ERR_POS, fname);
	if (unneed_conversion) {
	  FREAD (this_c, sizeof(LIBFB_Info_Call),
		 pu_hdr_entry->pu_num_call_entries, fp, ERR_READ, fname);

	} else {
	  for (i = 0; i < pu_hdr_entry->pu_num_call_entries; i++) {
	    fb_reader_Info_Read(&(this_c[i]), LIBFB_Info_Call_Type, fp, (char *)fname);
	  }
	}  
      }

    void fb_reader_Get_Sizes_Of_Switches(INT32 *this_c, Pu_Hdr* pu_hdr_entry, long pu_ofst,
						   FILE *fp, char *fname)
      {
	int i;
	FSEEK(fp, pu_ofst + pu_hdr_entry->pu_switch_target_offset, SEEK_SET, ERR_POS, fname);
	if (unneed_conversion) {
	  FREAD (this_c, sizeof(INT32),
		 pu_hdr_entry->pu_num_switch_entries, fp, ERR_READ, fname);

	} else {
	  for (i = 0; i < pu_hdr_entry->pu_num_switch_entries; i++) {
	    INT32_Read(&(this_c[i]), fp, (char *)fname);
	  }
	}  
      }

    void fb_reader_Get_Sizes_Of_CGotos(INT32 *this_c, Pu_Hdr* pu_hdr_entry, long pu_ofst,
						  FILE *fp, char *fname)
      {
	int i;
	FSEEK(fp, pu_ofst + pu_hdr_entry->pu_cgoto_target_offset, SEEK_SET, ERR_POS, fname);
	if (unneed_conversion) {
	  FREAD (this_c, sizeof(INT32),
		 pu_hdr_entry->pu_num_cgoto_entries, fp, ERR_READ, fname);

	} else {
	  for (i = 0; i < pu_hdr_entry->pu_num_cgoto_entries; i++) {
	    INT32_Read(&(this_c[i]), fp, (char *)fname);
	  }
	}  
      }

    void fb_reader_Info_Switch_begin(Pu_Hdr pu_hdr_entry, long pu_ofst,
					FILE *fp, char *fname)
      {
	FSEEK(fp, pu_ofst + pu_hdr_entry.pu_switch_offset, SEEK_SET, ERR_POS, fname);
      }

    void fb_reader_Info_CGoto_begin(Pu_Hdr pu_hdr_entry, long pu_ofst,
				       FILE *fp, char *fname)
      {
	FSEEK(fp, pu_ofst + pu_hdr_entry.pu_cgoto_offset, SEEK_SET, ERR_POS, fname);
      }


    // Read the string table that holds the names of PU's;
    // It is assumed that the caller has allocated the 
    // necessary storage in str_table.
    void fb_reader_Str_Table(FILE *fp, char *fname, Fb_Hdr* fb_hdr, char *str_table)
      {
	FSEEK(fp,fb_hdr->fb_str_table_offset, SEEK_SET, ERR_POS, fname);

	FREAD(str_table, sizeof(char), fb_hdr->fb_str_table_size, fp, ERR_READ,
	      fname);
      }


#ifdef __cplusplus
  }
#endif /* __cplusplus */
  // ====================================================================
