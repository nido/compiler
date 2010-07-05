#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>

#include "air.h"
#include "air_utils.h"
#include "parser.h"
#include "type_defs.h"

#include "ext_engine.h"

#include "targ_isa_registers.h"
#include "targ_isa_lits.h"
#include "targ_isa_print.h"
#include "targ_abi_properties.h"
#include "ti_asm.h"

#define MAX_SYMBOLS 256
char *symbols[MAX_SYMBOLS];
int symbol_id = 0;

void
pm_fatal_error( const char *format, ... ) {
  va_list args;

  va_start(args, format);
  fprintf(stderr,"Fatal error: ");
  vfprintf(stderr,format,args);
  va_end(args);
  exit(-1);
}

void
pm_error( const char *format, ... ) {
  va_list args;

  va_start(args, format);
  fprintf(stderr,"Error: ");
  vfprintf(stderr,format,args);
  va_end(args);
}

void
pm_warning( const char *format, ... ) {
  va_list args;

  va_start(args, format);
  fprintf(stderr,"Warning: ");
  vfprintf(stderr,format,args);
  va_end(args);
}

PARSER_ReturnT
parse_expression( char *current_pos, char **endptr, PARSER_u32T lit_class,
		  PARSER_boolT *is_reloc, void **sym_id, 
		  void **symop_id, PARSER_u64T *value, void **expr) {
  int is_a_value = 1;
  *sym_id = 0;
  *symop_id = 0;
  *expr = 0;
  *value = strtoll(current_pos,endptr,0);
  
  if(current_pos == *endptr) {
    char *str = 0;
    while(('a' <= **endptr && **endptr <= 'z') ||
	  ('A' <= **endptr && **endptr <= 'Z') ||
	  ('0' <= **endptr && **endptr <= '9') ||
	  **endptr == '_') {
      (*endptr)++;
    }
    if(current_pos == *endptr) {
      return PARSER_ERR_NONFATAL;
    }
    is_a_value = 0;
    if(symbol_id >= MAX_SYMBOLS) {
      AIR_error("Internal","IE_0xx","Max symbol overflow (%d)\n",MAX_SYMBOLS); exit(-1);
    }
    symbols[symbol_id] = (char*)malloc(*endptr - current_pos + 1);
    strncpy(symbols[symbol_id],current_pos,*endptr - current_pos);
    symbols[symbol_id + *endptr - current_pos] = '\0';
    *sym_id = (void*)symbol_id++;
    *value = 0;
    *is_reloc = 1;
  }

  if(is_a_value && !ISA_LC_Value_In_Class (*value, lit_class)) {
#ifdef PARSER_DEBUG
    printf("%s: value %lld does not fit in literal range\n",__FUNCTION__,*value);
#endif
    return PARSER_ERR_NONFATAL;
  }
  return PARSER_OK;
}

PARSER_ReturnT
parse_register( char *current_pos, char **endptr, PARSER_u32T reg_class, PARSER_u32T reg_subclass, PARSER_u32T *regid ) {
  const ISA_REGISTER_CLASS_INFO *rc_info = ISA_REGISTER_CLASS_Info((ISA_REGISTER_CLASS)reg_class);
  const ISA_REGISTER_SUBCLASS_INFO *rsc_info = NULL;
  const char *rc_reg_name = NULL;
  const char *reg_name = 0;
  int reg_name_len = 0;
  int reg_id = 0;
  int find_reg = 0;

  *endptr = current_pos;

  if(reg_subclass != ISA_REGISTER_SUBCLASS_UNDEFINED) {
    rsc_info = ISA_REGISTER_SUBCLASS_Info((ISA_REGISTER_SUBCLASS)reg_subclass);

    /* Start by the last one and expect getting the longest name first ($r16 before $r1) ... */
    reg_id = ISA_REGISTER_SUBCLASS_INFO_Count(rsc_info) - 1;

    while(!find_reg && reg_id >= 0) {
      reg_name = ISA_REGISTER_SUBCLASS_INFO_Reg_Name(rsc_info,reg_id);
      reg_name_len = strlen(reg_name);
      if(strncasecmp(reg_name,current_pos,reg_name_len) == 0) {
	find_reg = 1;
      }
      else {
	reg_id--;
      }
    }
  }
  else {
    reg_id = ISA_REGISTER_CLASS_INFO_Last_Reg(rc_info);
    while(!find_reg && reg_id >= ISA_REGISTER_CLASS_INFO_First_Reg(rc_info)) {
      reg_name = ABI_PROPERTY_Reg_Name((ISA_REGISTER_CLASS)reg_class,reg_id);
      reg_name_len = strlen(reg_name);
      if(strncasecmp(reg_name,current_pos,reg_name_len) == 0) {
	find_reg = 1;
      }
      else {
	reg_name = ISA_REGISTER_CLASS_INFO_Reg_Name(rc_info,reg_id);
	reg_name_len = strlen(reg_name);
	if(strncasecmp(reg_name,current_pos,reg_name_len) == 0) {
	  find_reg = 1;
	}
	else {
	  reg_id--;
	}
      }
    }
  }

  if(!find_reg) {
    return PARSER_ERR_NONFATAL;
  }

  *regid = reg_id;

  *endptr += reg_name_len;

  return PARSER_OK;
}

/* ====================================================================
 *   r_value
 *
 *   Returns TN value depending of its type.
 * ====================================================================
 */
static INT64
r_value( AIR_TN *tn )
{
  if (Is_AIR_TN_expression(tn)) {
    switch(Get_AIR_TN_exp_kind(tn)) {
    case AIR_Expression_immediate:
      return Get_AIR_TN_exp_imm_val(tn);
      break;
    case AIR_Expression_reloc:
      return Get_AIR_TN_exp_reloc_val(tn);
      break;
    case AIR_Expression_regmask:
      Get_AIR_TN_exp_regmask_mask(tn);
      return;
    default:
      AIR_error("Internal","IE_004","Expression not yet printable (%d)\n",Get_AIR_TN_exp_kind(tn)); exit(-1);
    }
  }
  else if(Is_AIR_TN_register(tn)) {
     return Get_AIR_TN_reg_regnum(tn);
  }
  return 0;
}


typedef mUINT32 REGISTER;
#define REGISTER_UNDEFINED          ((REGISTER) 0)

#define REGISTER_CLASS_reg_name(x)				\
				(ISA_REGISTER_CLASS_info[x].reg_name)
#define REGISTER_name(rclass,reg)				\
				(REGISTER_CLASS_reg_name(rclass)[reg])
/* ====================================================================
 *   r_assemble_opnd
 * ====================================================================
 */
static char*
r_assemble_opnd (
  AIR_OP *instr,
  int             i,
  char            *buf,
  int             *buf_idx
)
{
  char *buf_ptr = &buf[*buf_idx];
  AIR_TN *opnd = Get_AIR_OP_operandn(instr,i);

  if(i>=ISA_OPERAND_INFO_Operands(ISA_OPERAND_Info(Get_AIR_OP_TOP(instr)))) {
    AIR_fatal_error("","FE_000","Operand %d does not exist for %s",i,TOP_Name(Get_AIR_OP_TOP(instr)));
  }

  if(opnd == 0) {
    AIR_fatal_error("","FE_001","Operand %d not initialized for %s",i,TOP_Name(Get_AIR_OP_TOP(instr)));
  }

  if (Is_AIR_TN_expression(opnd)) {
    switch(Get_AIR_TN_exp_kind(opnd)) {
    case AIR_Expression_immediate:
      *buf_idx += sprintf(buf_ptr,"%lld",Get_AIR_TN_exp_imm_val(opnd)) + 1;
      break;
    case AIR_Expression_reloc:
      *buf_idx += sprintf(buf_ptr,"%s",symbols[(int)Get_AIR_TN_exp_reloc_symid(opnd)]) + 1;
      break;
    case AIR_Expression_regmask:
    default:
      AIR_error("Internal","IE_004","Expression not yet printable (%d)\n",Get_AIR_TN_exp_kind(opnd)); exit(-1);
    }
  }
  else if(Is_AIR_TN_register(opnd)) {
    const char *rname;
    ISA_REGISTER_CLASS rc = Get_AIR_TN_reg_regclass(opnd);
    const ISA_REGISTER_CLASS_INFO *rc_info = ISA_REGISTER_CLASS_Info(rc);
    REGISTER reg = Get_AIR_TN_reg_regnum(opnd);

    ISA_REGISTER_SUBCLASS sc = Get_AIR_TN_reg_subclass(opnd);
    
    if(sc != ISA_REGISTER_SUBCLASS_UNDEFINED) {
      const ISA_REGISTER_SUBCLASS_INFO *rsc_info = ISA_REGISTER_SUBCLASS_Info(sc);
      rname = ISA_REGISTER_SUBCLASS_INFO_Reg_Name(rsc_info,reg);
    }
    else if((rname = ABI_PROPERTY_Reg_Name(rc,reg)) != 0) { }
    else {
      rname = ISA_REGISTER_CLASS_INFO_Reg_Name(rc_info,reg);
    }

    *buf_idx += sprintf(buf_ptr,"%s",rname) + 1;
  }

  return buf_ptr;
}

/* ====================================================================
 *   r_assemble_result
 * ====================================================================
 */
static char*
r_assemble_result (
  AIR_OP *instr,
  int i,
  char            *buf,
  int             *buf_idx
)
{
  char *buf_ptr = &buf[*buf_idx];
  AIR_TN *res = Get_AIR_OP_resultn(instr,i);
  const char *rname;

  if(i>=ISA_OPERAND_INFO_Results(ISA_OPERAND_Info(Get_AIR_OP_TOP(instr)))) {
    AIR_fatal_error("","FE_002","Result %d does not exist for %s",i,TOP_Name(Get_AIR_OP_TOP(instr)));
  }

  if(res == 0) {
    AIR_fatal_error("","FE_003","Result %d not initialized for %s",i,TOP_Name(Get_AIR_OP_TOP(instr)));
  }

  ISA_REGISTER_SUBCLASS sc = Get_AIR_TN_reg_subclass(res);
  ISA_REGISTER_CLASS rc = Get_AIR_TN_reg_regclass(res);
  const ISA_REGISTER_CLASS_INFO *rc_info = ISA_REGISTER_CLASS_Info(rc);
  REGISTER reg = Get_AIR_TN_reg_regnum(res);

  if(sc != ISA_REGISTER_SUBCLASS_UNDEFINED) {
    const ISA_REGISTER_SUBCLASS_INFO *rsc_info = ISA_REGISTER_SUBCLASS_Info(sc);
    rname = ISA_REGISTER_SUBCLASS_INFO_Reg_Name(rsc_info,reg);
  }
  else if((rname = ABI_PROPERTY_Reg_Name(rc,reg)) != 0) { }
  else {
    rname = ISA_REGISTER_CLASS_INFO_Reg_Name(rc_info,reg);
  }

  *buf_idx += sprintf(buf_ptr,"%s",rname) + 1;
  return buf_ptr;
}

static void r_assemble_list ( FILE *file, AIR_OPS *ops ) {
  ISA_PRINT_OPND_INFO result[ISA_OPERAND_max_results];
  ISA_PRINT_OPND_INFO opnd[ISA_OPERAND_max_operands];
  char buf[65536];
  int buf_idx = 0;
  int i;
  AIR_OP *op;

  FOR_ALL_AIR_OPS_OPs(ops,op) {
    fprintf(stdout,"'");
    for (i = 0; i < Get_AIR_OP_nb_operands(op); i++) {
      AIR_TN *tn = Get_AIR_OP_operandn(op,i);
      opnd[i].name = r_assemble_opnd(op, i, buf, &buf_idx);
      if(buf_idx >= 65536) { AIR_error("Internal","IE_005","Buffer overflow during operand print\n"); exit(-1); }
      opnd[i].value = r_value(tn);
    }
    
    for (i = 0; i < Get_AIR_OP_nb_results(op); i++) {
      AIR_TN *tn = Get_AIR_OP_resultn(op,i);
      result[i].name = r_assemble_result (op, i, buf, &buf_idx);
      if(buf_idx >= 65536) { AIR_error("Internal","IE_006","Buffer overflow during result print\n"); exit(-1); }
      result[i].value = r_value(tn);
    }
    
    TI_ASM_Print_Inst(Get_AIR_OP_TOP(op), result, opnd, file);
    fprintf(stdout,"'\t=> TOP: %s\n",TOP_Name(Get_AIR_OP_TOP(op)));
  }
}

static void
print_syntax_error( char *line ) {
  int errorIndex = PARSER_error_index();
  int i = 0;
  char *str_buf = (char*)malloc(strlen(line) + 1);
  fprintf(stderr,"Syntax error: '%s'\n",line);
  fprintf(stderr,"               ");
  while(i < errorIndex) {
    if(line[i] == '\t') {
      str_buf[i++] = '\t';
    }
    else {
      str_buf[i++] = ' ';
    }
  }
  str_buf[i] = '\0';
  fprintf(stderr,"%s^\n",str_buf);
  free(str_buf);
}

static PARSER_s32T
getline( char **line, PARSER_u32T *len, FILE *file ) {
  int cur_char = '\0';
  char *cur_ptr = *line;
  int end_of_line = 0;
  if(*line == 0) {
    *len = 512;
    *line = (char*)malloc(*len);
    if(*line == 0) {
      AIR_error("Internal","IE_007","Unable to allocate memory: %s\n",strerror(errno));
      return -1;
    }
    cur_ptr = *line;
  }
  while(!end_of_line && (cur_char = fgetc(file)) != EOF) {
    if(cur_ptr - *line >= *len) {
      char *tmp = realloc(*line,*len*2);
      if(tmp == 0) {
	AIR_error("","IE_008","Unable to reallocate memory: %s\n",strerror(errno));
	return -1;
      }
      if(tmp != *line) {
	int i;
	for(i=0;i<*len;i++) {
	  tmp[i] = (*line)[i];
	}
	*line = tmp;
	cur_ptr = &(*line)[*len];
      }
      *len *= 2;
    }
    *cur_ptr++ = cur_char;
    if(cur_char == '\n') {
      end_of_line = 1;
    }
  }
  if(cur_char == EOF) { return -1; }
  *cur_ptr = '\0';
  return cur_ptr - *line;
}


static PARSER_ReturnT
parse_file( char *file_name, AIR_OPS *ops ) {
  FILE *file = fopen(file_name,"r");
  char * line = NULL;
  size_t len = 0;
  ssize_t read;
  AIR_OP *instrs = NULL;
  PARSER_ReturnT ret;

  if(file == NULL) {
    AIR_error("Internal","IE_009","Unable to open file %s: %s\n",file_name,strerror(errno));
    exit(-1);
  }

  while((read = getline(&line, &len, file)) != -1) {
    line[read-1] = '\0';
#if 0
    fprintf(stderr,"-> %s\n",line);
#endif
    if(read > 1 && line[0] != '#') {
      if((ret = PARSER_parse(line,ops)) != PARSER_OK) {
	print_syntax_error(line);
	return ret;
      }
    }
  }
  if(line) {
    free(line);
  }

  fclose(file);
  return ret;
}

static void
print_usage() {
  const char *usage_str = 
    "  -h         : Print help.\n"
    "  -f <file>  : Parse given file containing assembly instructions separated by carriage return.\n"
    "  -i <instr> : Parse given assembly instruction.\n"
    "  -e <extension library path> : Load a parser extension shared library.\n"
    "\n";
  fprintf(stdout,"%s",usage_str);
  exit(0);
}

int main (int argc, char *argv[])
{   
  PARSER_ReturnT ret;
  char *instruction = "  addu R15, R15, 4";
  int i;

  AIR_OPS ops = AIR_OPS_EMPTY;
  AIR_init();
  ret = PARSER_Initialize(&parse_register,&parse_expression,
			  &pm_fatal_error,&pm_error,&pm_warning,
			  "./parser");
  ABI_PROPERTIES_ABI_Value = 0;
  ABI_PROPERTIES_Initialize();

  if(ret != PARSER_OK) { return ret; }
  if(argc >= 2) {
    int arg_idx = 1;
    while(arg_idx < argc) {
      if(argv[arg_idx][0] == '-') {
	switch(argv[arg_idx][1]) {
	case 'e':
	  if(argc <= (arg_idx + 1)) {
	    AIR_error("","IE_010","Must specify a parser extension library file after '-e'\n");
	    print_usage();
	  }
	  arg_idx++;
	  PARSER_ConnectParser(argv[arg_idx]);
	  break;
	case 'f':
	  if(argc <= (arg_idx + 1)) {
	    AIR_error("Internal","IE_011","Must specify a assembly file after '-f'\n");
	    print_usage();
	  }
	  arg_idx++;
	  ret = parse_file(argv[arg_idx],&ops);
	  break;
	case 'h':
	  print_usage();
	  break;
	case 'i':
	  if(argc <= (arg_idx + 1)) {
	    AIR_error("Internal","IE_012","Must specify an instruction after '-i'\n");
	    print_usage();
	  }
	  arg_idx++;
	  ret = PARSER_parse(argv[arg_idx],&ops);
	  if(ret != PARSER_OK) {
	    print_syntax_error(argv[arg_idx]);
	  }
	  break;
	default:
	  print_usage();
	}
	arg_idx++;
      }
      else {
	print_usage();
      }
    }
  }
  else {
    printf("By default, parse \"%s\". You can specify other instruction as argument.\n",instruction);
    ret = PARSER_parse(instruction,&ops);
    if(ret != PARSER_OK) {
      print_syntax_error(instruction);
    }
  }
  if(ret == PARSER_OK) {
#ifdef PARSER_DEBUG
    AIR_check_ops(stderr,&ops,1);
#else
    AIR_check_ops(stderr,&ops,0);
#endif
    r_assemble_list ( stdout, &ops );
#if 0
    AIR_dump_ops(stdout,&ops);
#endif
#ifdef PARSER_DEBUG
    printf("Parsing OK\n");
#endif
  }

  for(i=0;i<symbol_id;i++) {
    free(symbols[i]);
  }

  PARSER_DisconnectParsers();
  AIR_end();
  PARSER_Finalize();
  return ret;
}
