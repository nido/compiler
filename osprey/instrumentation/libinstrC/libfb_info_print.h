#ifndef libfb_info_print_INCLUDED
#define libfb_info_print_INCLUDED
// ====================================================================
// Some print functions

/* #ifdef INSTR_DEBUG */
inline void INT64_Print( INT64 this, FILE *fp ) {
  fprintf(fp, "%d", (int)this);
}

inline void LIBFB_Info_Invoke_Print( LIBFB_Info_Invoke *this, FILE *fp ) {
  fprintf( fp, "FB---> invoke = " );
  INT64_Print( this->freq_invoke, fp);
}

inline void LIBFB_Info_Branch_Print( LIBFB_Info_Branch *this, FILE *fp ) {
    fprintf( fp, "FB---> taken = " );
    INT64_Print( this->freq_taken, fp );
    fprintf( fp, ", not_taken = " );
    INT64_Print( this->freq_not_taken, fp );
  }

inline  void LIBFB_Info_Loop_Print( LIBFB_Info_Loop *this, FILE *fp ) {
  fprintf( fp, "FB---> total inv = " );
  INT64_Print( this->invocation_count, fp );
  fprintf( fp, ", total trip count = " );
  INT64_Print( this->total_trip_count, fp );
  fprintf( fp, ", trip count of last inv = " );
  INT64_Print( this->last_trip_count, fp );
  fprintf( fp, ", min trip count = " );
  INT64_Print( this->min_trip_count, fp );
  fprintf( fp, "\n max trip count = " );
  INT64_Print( this->max_trip_count, fp );
  fprintf( fp, ", nb times 0 trip = " );
  INT64_Print( this->num_zero_trips, fp );
}

inline  void LIBFB_Info_Circuit_Print( LIBFB_Info_Circuit *this, FILE *fp ) {
    fprintf( fp, "FB---> left = " );
    INT64_Print( this->freq_left, fp );
    fprintf( fp, ", right = " );
    INT64_Print( this->freq_right, fp );
    fprintf( fp, ", neither = " );
    INT64_Print( this->freq_neither, fp );
  }

inline  void LIBFB_Info_Call_Print( LIBFB_Info_Call *this, FILE *fp ) {
    fprintf( fp, "FB---> entry = " );
    INT64_Print( this->freq_entry, fp );
    fprintf( fp, ", exit = " );
    INT64_Print( this->freq_exit, fp );
  }
/* #endif // INSTR_DEBUG */
#endif // libfb_info_print_INCLUDED
