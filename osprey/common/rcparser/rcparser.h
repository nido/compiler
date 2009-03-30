#ifndef _RCPARSER_H_
#define _RCPARSER_H_

typedef struct {
   char * Name;
   char * Architecture;
   char * ExtHwType;
   char * CmpLibPath;
   char * CmpLibName;
   char * AsmLibPath;
   char * AsmLibName;
   char * LdLibPath;
   char * LdLibName;
   char * DisasmLibPath;
   char * DisasmLibName;
   char * ToolsetLibPath;
   char * ToolsetLibName;
   char * CmpTargOpt;
   char * Defines;
   char * Includes;
   char * HelpMsg;
} RCparser_ExtensionInfoT;

typedef enum {
  RCPARSER_OPEN64=1,
  RCPARSER_FLEXCC=2
} RCparser_rctypeT;

extern void RCparser_Init ( RCparser_rctypeT rctype );
extern void RCparser_CreatePatch ( char *lib_path, char *incpath, 
                                   char *ext_name, char *ext_long_name, 
                                   int remove, int Multiplier, 
                                   int CompilerDll, char * ext_arch, char * ext_hwtype );
extern void RCparser_parse ( char * infilename, char * ModuleName );
extern void RCparser_patch ( char * outfilename );
extern int RCparser_getextnr ( void );
extern RCparser_ExtensionInfoT * RCparser_getextinfo ( int index, char * ModuleName );
extern void RCparser_printextinfo (RCparser_ExtensionInfoT * extinfo );

#define FOREACH_EXTENSION(index) for(index=0; index < RCparser_getextnr(); index++)

#endif
