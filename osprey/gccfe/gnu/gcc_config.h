

#ifndef GCC_CONFIG_H_included
#define GCC_CONFIG_H_included
#ifdef TARG_ST
//This file contaons all variables and types needed to transform some
//#define in the config/target/target.h file

//TB: for extension, make ADDITIONAL_REGISTER_NAMES a real array
typedef struct {  char *name;  int number; } gcc_register_map_t;
#endif //TARG_ST
#endif// GCC_CONFIG_H_included
