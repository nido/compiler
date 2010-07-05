#ifndef libfb_info_print_INCLUDED
#define libfb_info_print_INCLUDED
// ====================================================================
// Some print functions

/* #ifdef INSTR_DEBUG */
static inline void INT64_Print( INT64 this, FILE *fp ) {
  fprintf(fp, "%d", (int)this);
}

static inline void TNV_Print(LIBFB_TNV tnv, FILE *fp ) {
  fprintf(fp, "id(%u), exec_counter(%llu), flag(%d), zero_std_counter(%llu), (values(counters)=( 0x%llx(%llu), 0x%llx(%llu), 0x%llx(%llu), 0x%llx(%llu), 0x%llx(%llu), 0x%llx(%llu), 0x%llx(%llu), 0x%llx(%llu), 0x%llx(%llu), 0x%llx(%llu) ) )\n",
	  tnv._id, tnv._exec_counter, tnv._flag, tnv._zero_std_counter,
	  tnv._values[0], tnv._counters[0] , tnv._values[1], tnv._counters[1], 
	  tnv._values[2], tnv._counters[2], tnv._values[3], tnv._counters[3], tnv._values[4], tnv._counters[4], 
	  tnv._values[5], tnv._counters[5], tnv._values[6], tnv._counters[6], tnv._values[7], tnv._counters[7], 
	  tnv._values[8], tnv._counters[8], tnv._values[9], tnv._counters[9]  
	  );
}


static inline void* LIBFB_Info_Invoke_Print( LIBFB_Info_Invoke *this, FILE *fp ) {
  fprintf( fp, "FB---> invoke = " );
  INT64_Print( this->freq_invoke, fp);
  return this+1;
}

static inline void* LIBFB_Info_Branch_Print( LIBFB_Info_Branch *this, FILE *fp ) {
  fprintf( fp, "FB---> taken = " );
  INT64_Print( this->freq_taken, fp );
  fprintf( fp, ", not_taken = " );
  INT64_Print( this->freq_not_taken, fp );
  return this+1;
}

static inline void* LIBFB_Info_Loop_Print( LIBFB_Info_Loop *this, FILE *fp ) {
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
  return this+1;
}

static inline void* LIBFB_Info_Circuit_Print( LIBFB_Info_Circuit *this, FILE *fp ) {
  fprintf( fp, "FB---> left = " );
  INT64_Print( this->freq_left, fp );
  fprintf( fp, ", right = " );
  INT64_Print( this->freq_right, fp );
  fprintf( fp, ", neither = " );
  INT64_Print( this->freq_neither, fp );
  return this+1;
}

static inline void* LIBFB_Info_Call_Print( LIBFB_Info_Call *this, FILE *fp ) {
  fprintf( fp, "FB---> entry = " );
  INT64_Print( this->freq_entry, fp );
  fprintf( fp, ", exit = " );
  INT64_Print( this->freq_exit, fp );
  return this+1;
}

static inline void* LIBFB_Info_Icall_Print( LIBFB_Info_Icall *this, FILE *fp ) {
  fprintf( fp, "FB---> tnv = " );
  TNV_Print( this->tnv, fp );
  return this+1;
}

static inline  void* LIBFB_Info_Value_Print( LIBFB_Info_Value *this, FILE *fp ) {
  int i;
  fprintf( fp, "execution counter: %d\n", (int)this->exe_counter );
  for( i = 0; i < this->num_values; i++ ){
    fprintf( fp, "value %lld\t freq %lld\n", this->value[i], this->freq[i] );
  }
  return this+1;
}

static inline  void* LIBFB_Info_Value_FP_Bin_Print( LIBFB_Info_Value_FP_Bin *this, FILE *fp ) {
  fprintf( fp, "execution counter: %lld\n", this->exe_counter );
  fprintf( fp, "operand 0 zero counter: %lld\n", this->zopnd0 );
  fprintf( fp, "operand 1 zero counter: %lld\n", this->zopnd1 );
  fprintf( fp, "operand 0 one counter: %lld\n", this->uopnd0 );
  fprintf( fp, "operand 1 one counter: %lld\n", this->uopnd1 );
  return this+1;
}
/* #endif // INSTR_DEBUG */
#endif // libfb_info_print_INCLUDED
