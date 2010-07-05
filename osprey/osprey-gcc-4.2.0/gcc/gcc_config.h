

#ifndef GCC_CONFIG_H_included
#define GCC_CONFIG_H_included
//This file contaons all variables and types needed to transform some
//#define in the config/target/target.h file

//TB: for extension, make ADDITIONAL_REGISTER_NAMES a real array
typedef struct {  char *name;  int number; int disabled; } gcc_register_map_t;
#endif// GCC_CONFIG_H_included
