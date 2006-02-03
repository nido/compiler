#include "libfb_writer.h"
#include "profile_error.h"
/* static char* ERR_POS = "Error in positioning within %s"; */
static char* ERR_WRITE = "Error in writing to %s";
/* static char* ERR_READ = "Error in reading from %s"; */
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

/*****************************************************************

                    WRITER FUNCTIONS

*****************************************************************/

/* inline void int_Write(int this_c, FILE *fp, char *output_filename){ */
/*   char ptr[4]; */
/*   union u { */
/*     int f; */
/*     char b[4]; */
/*   } p; */
/*   p.f = this_c; */

/*   ptr[0] = p.b[3]; */
/*   ptr[1] = p.b[2]; */
/*   ptr[2] = p.b[1]; */
/*   ptr[3] = p.b[0]; */

/*   FWRITE(ptr, sizeof(char), 4, fp, ERR_WRITE, output_filename); */
/* } */


/* inline void float_Write(float this_c, FILE *fp, char *output_filename){ */
/*   char ptr[4]; */
/*   union u { */
/*     float f; */
/*     char b[4]; */
/*   } p; */
/*   p.f = this_c; */

/*   ptr[0] = p.b[3]; */
/*   ptr[1] = p.b[2]; */
/*   ptr[2] = p.b[1]; */
/*   ptr[3] = p.b[0]; */

/*   FWRITE(ptr, sizeof(char), 4, fp, ERR_WRITE, output_filename); */
/* } */

void INT32_Write(INT32 this_c, FILE *fp, char *fname){
  FWRITE (&this_c, sizeof(INT32), 1, fp, ERR_WRITE, fname);
}

static void ULONG_Write(ULONG this_c, FILE *fp, char *fname)
{
  FWRITE (&this_c, sizeof(ULONG), 1, fp, ERR_WRITE, fname);
}

/* static void BOOL_Write(BOOL this_c, FILE *fp, char *fname) { */
/*   FWRITE (&this_c, sizeof(BOOL), 1, fp, ERR_WRITE, fname); */
/* } */

static void enum_Write(int this_c, FILE *fp, char *fname) {
  FWRITE (&this_c, sizeof(INT32), 1, fp, ERR_WRITE, fname);
}

static void INT64_Write(INT64 this_c, FILE *fp, char *fname){
  FWRITE (&this_c, sizeof(INT64), 1, fp, ERR_WRITE, fname);
}

void INT64_list_Write(const INT64 *this_c, int nitems, FILE *fp, char *fname){
/*   int i; */
  FWRITE (&this_c[0], sizeof(INT64), nitems, fp, ERR_WRITE, fname);
/*   for (i = 0; i < nitems; i++) { */
/*     enum_Write(this_c[i]._type, fp, fname); */
/*     float_Write(this_c[i]._value, fp, fname); */
/*   } */
}

static void LIBFB_Info_Invoke_Write(LIBFB_Info_Invoke *this_c, FILE *fp, char *fname) {
  INT64_Write(this_c->freq_invoke, fp, fname);
}

static void LIBFB_Info_Branch_Write( LIBFB_Info_Branch *this_c, FILE *fp, char *fname) {
    INT64_Write( this_c->freq_taken, fp, fname);
    INT64_Write( this_c->freq_not_taken, fp, fname);
}

static void LIBFB_Info_Loop_Write( LIBFB_Info_Loop *this_c, FILE *fp, char *fname) {
  INT64_Write( this_c->invocation_count, fp, fname);
  INT64_Write( this_c->total_trip_count, fp, fname);
  INT64_Write( this_c->last_trip_count, fp, fname);
  INT64_Write( this_c->min_trip_count, fp, fname);
  INT64_Write( this_c->max_trip_count, fp, fname);
  INT64_Write( this_c->num_zero_trips, fp, fname);
}

static void LIBFB_Info_Circuit_Write( LIBFB_Info_Circuit *this_c, FILE *fp, char *fname) {
  INT64_Write( this_c->freq_left, fp, fname);
  INT64_Write( this_c->freq_right, fp, fname);
  INT64_Write( this_c->freq_neither, fp, fname);
}

static void LIBFB_Info_Call_Write( LIBFB_Info_Call *this_c, FILE *fp, char *fname) {
  INT64_Write( this_c->freq_entry, fp, fname);
  INT64_Write( this_c->freq_exit, fp, fname);
}

void Fb_Hdr_Write(const Fb_Hdr *this_c, FILE *fp, char *fname){

  FWRITE(this_c->fb_ident, FB_NIDENT, 1, fp, ERR_WRITE, fname);
  ULONG_Write( this_c->fb_version, fp, fname);
  ULONG_Write( this_c->fb_profile_offset, fp, fname);
  ULONG_Write( this_c->fb_pu_hdr_offset, fp, fname); 
  ULONG_Write( this_c->fb_pu_hdr_ent_size, fp, fname);
  ULONG_Write( this_c->fb_pu_hdr_num, fp, fname);     
  ULONG_Write( this_c->fb_str_table_offset, fp, fname);
  ULONG_Write( this_c->fb_str_table_size, fp, fname);
  enum_Write( this_c->phase_num, fp, fname);
}

void Pu_Hdr_Write(const Pu_Hdr *this_c, FILE *fp, char *fname){
  INT32_Write( this_c->pu_checksum, fp, fname);
  ULONG_Write( this_c->pu_name_index, fp, fname);
  ULONG_Write( this_c->pu_file_offset, fp, fname);
  ULONG_Write( this_c->pu_inv_offset, fp, fname);
  ULONG_Write( this_c->pu_num_inv_entries, fp, fname);
  ULONG_Write( this_c->pu_br_offset, fp, fname);
  ULONG_Write( this_c->pu_num_br_entries, fp, fname);
  ULONG_Write( this_c->pu_switch_offset, fp, fname);
  ULONG_Write( this_c->pu_switch_target_offset, fp, fname);
  ULONG_Write( this_c->pu_num_switch_entries, fp, fname);
  ULONG_Write( this_c->pu_cgoto_offset, fp, fname);
  ULONG_Write( this_c->pu_cgoto_target_offset, fp, fname);
  ULONG_Write( this_c->pu_num_cgoto_entries, fp, fname);
  ULONG_Write( this_c->pu_loop_offset, fp, fname);
  ULONG_Write( this_c->pu_num_loop_entries, fp, fname);
  ULONG_Write( this_c->pu_scircuit_offset, fp, fname);
  ULONG_Write( this_c->pu_num_scircuit_entries, fp, fname);
  ULONG_Write( this_c->pu_call_offset, fp, fname);
  ULONG_Write( this_c->pu_num_call_entries, fp, fname);
}

typedef  void (*dump_func_type)(void *, FILE*, char*);

static void *FB_Info_Type_Write(FB_Info_Type type)
{
  switch (type) {
  case LIBFB_Info_Invoke_Type:
    return (void *)&LIBFB_Info_Invoke_Write;
  case LIBFB_Info_Branch_Type:
    return (void *)&LIBFB_Info_Branch_Write;
  case LIBFB_Info_Circuit_Type:
    return (void *)&LIBFB_Info_Circuit_Write;
  case LIBFB_Info_Call_Type:
    return (void *)&LIBFB_Info_Call_Write;
  case LIBFB_Info_Loop_Type:
    return (void *)&LIBFB_Info_Loop_Write;
  default:
    profile_error("Unexpexted Type","");
    return 0;
    break;
  }
}

void FB_Info_Write(const void *this_c, FB_Info_Type type, FILE *fp, char *fname)
{
  dump_func_type dump_func = FB_Info_Type_Write(type); 

  dump_func((void *)this_c, fp, fname);
}

