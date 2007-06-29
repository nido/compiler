//-*-c++-*-
// ====================================================================
// ====================================================================
//
// Module: profile_aux.h
//
// ====================================================================
//
// Description:
//
// These data structures are used to store frequency counts during
// the run of an instrumented program unit.
//
// ====================================================================
// ====================================================================


#ifndef profile_aux_print_INCLUDED
#define profile_aux_print_INCLUDED

#include "libfb_info_print.h"

inline  void Compgoto_Profile_Print( Compgoto_Profile *this, FILE *fp ) {
  size_t size, i;
  fprintf( fp, "COMPGOTO---> targets_profile = " );
  if (!this->targets_profile) return;
  size = this->targets_profile_size;
  for (i = 0; i < size; i++) {
    fprintf(fp, "\tid = %d\t", i);
    INT64_Print(this->targets_profile[i], fp);
  }
}

inline  void Switch_Profile_Print( Switch_Profile *this, FILE *fp ) {
  size_t size, i;
  fprintf( fp, "SWITCH---> targets_profile = " );
  if (!this->targets_profile) return;
  size = this->targets_profile_size;
  for (i = 0; i < size; i++) {
    fprintf(fp, "\tid = %d\t", i);
    INT64_Print(this->targets_profile[i], fp);
  }
  fprintf( fp, "\n\t targets_case_value = " );
  if (!this->targets_case_value) return;
  for (i = 0; i < size - 1; i++) {
    fprintf(fp, "\tid = %d\t", i);
    INT64_Print(this->targets_case_value[i], fp);
  }
}

inline void *Type_Profile_Print(Type_Profile type)
{
  switch (type) {
  case Invoke_Type_Profile:
    return (void *)&LIBFB_Info_Invoke_Print;
  case Branch_Type_Profile:
    return (void *)&LIBFB_Info_Branch_Print;
  case Compgoto_Type_Profile:
    return (void *)&Switch_Profile_Print;
  case Short_Circuit_Type_Profile:
    return (void *)&LIBFB_Info_Circuit_Print;
  case Call_Type_Profile:
    return (void *)&LIBFB_Info_Call_Print;
  case Loop_Type_Profile:
    return (void *)&LIBFB_Info_Loop_Print;
  case Switch_Type_Profile :
    return (void *)&Switch_Profile_Print;
  case Icall_Type_Profile :
    return (void *)&LIBFB_Info_Icall_Print;
  case Value_Type_Profile :
    return (void *)&LIBFB_Info_Value_Print;
  case Value_FP_Bin_Type_Profile :
    return (void *)&LIBFB_Info_Value_FP_Bin_Print;
  default:
    profile_error("Unexpexted Type","");
    return 0;
    break;
  }
}

typedef  void* (*print_func_type)(void *, FILE*);

inline void Profile_Print(void *this, char *name, FILE *fp, Type_Profile type) {
  size_t size, i;
  void *ptr;
  print_func_type print_func;
  if (!this) return;
  size = ((LIBFB_Info_Invoke_Vector*)this)->size;
  print_func = (print_func_type) Type_Profile_Print(type);
  if (size != 0)
    fprintf (fp, "%s Profile: %d\n", name, size);
  ptr = &(((LIBFB_Info_Invoke_Vector*)this)->data[0]);
  for (i = 0; i < size; i++) {
    fprintf(fp, "\t%s id = %d\t", name, i);
    ptr = print_func(ptr, fp);
    fputc ('\n', fp);
  }
}

inline void PU_Profile_Handle_Print(PU_PROFILE_HANDLE this, FILE *fp) {
    fprintf( fp, "feedback data of %s (checksum %d) size: %d add0x%llx\n", this->pu_name, this->checksum, this->pu_size, this->runtime_fun_address);
    fprintf( fp, "Invoke:\n");
    Profile_Print(this->LIBFB_Info_Invoke_Table, this->pu_name, fp, Invoke_Type_Profile);
    fprintf( fp, "Branch:\n");
    Profile_Print( this->LIBFB_Info_Branch_Table, this->pu_name, fp, Branch_Type_Profile);
    fprintf( fp, "Switch:\n");
    Profile_Print( this->Switch_Profile_Table, this->pu_name, fp, Switch_Type_Profile);
    fprintf( fp, "Compgoto:\n");
    Profile_Print( this->Compgoto_Profile_Table, this->pu_name, fp, Compgoto_Type_Profile);
    fprintf( fp, "Loop:\n");
    Profile_Print( this->LIBFB_Info_Loop_Table, this->pu_name, fp, Loop_Type_Profile);
    fprintf( fp, "Short Circuit:\n");
    Profile_Print( this->LIBFB_Info_Circuit_Table, this->pu_name, fp, Short_Circuit_Type_Profile);
    fprintf( fp, "Call:\n");
    Profile_Print( this->LIBFB_Info_Call_Table, this->pu_name, fp, Call_Type_Profile);
    fprintf( fp, "Icall:\n");
    Profile_Print( this->LIBFB_Info_Icall_Table, this->pu_name, fp, Icall_Type_Profile);
    fprintf( fp, "Value:\n");
    Profile_Print( this->LIBFB_Info_Value_Table, this->pu_name, fp, Value_Type_Profile);
    fprintf( fp, "FP Bin Value:\n");
    Profile_Print( this->LIBFB_Info_Value_FP_Bin_Table, this->pu_name, fp, Value_FP_Bin_Type_Profile);
}

#endif /* profile_aux_print_INCLUDED */
