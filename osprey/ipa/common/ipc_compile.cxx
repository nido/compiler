/*

  Copyright (C) 2000, 2001 Silicon Graphics, Inc.  All Rights Reserved.

  This program is free software; you can redistribute it and/or modify it
  under the terms of version 2 of the GNU General Public License as
  published by the Free Software Foundation.

  This program is distributed in the hope that it would be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

  Further, this software is distributed without any warranty that it is
  free of the rightful claim of any third person regarding infringement 
  or the like.  Any license provided herein, whether implied or 
  otherwise, applies only to this software file.  Patent licenses, if 
  any, provided herein do not apply to combinations of this program with 
  other software, or any other product whatsoever.  

  You should have received a copy of the GNU General Public License along
  with this program; if not, write the Free Software Foundation, Inc., 59
  Temple Place - Suite 330, Boston MA 02111-1307, USA.

  Contact information:  Silicon Graphics, Inc., 1600 Amphitheatre Pky,
  Mountain View, CA 94043, or:

  http://www.sgi.com

  For further information regarding this notice, see:

  http://oss.sgi.com/projects/GenInfo/NoticeExplan

*/


#include "W_limits.h"                     // for PATH_MAX
#include "W_unistd.h"                     // for read(2)/write(2)
#include <fcntl.h>                      // for open(2)
#include <sys/types.h>
#include <sys/stat.h>                   // for chmod(2)
#include <string.h>
#include "W_bstring.h"			// for bzero
#include <W_errno.h>                      // for sys_errlist
#include <vector>                       // for STL vector container
// [HK]
#if __GNUC__ >=3
#include <ext/hash_map>
#else
#include <hash_map>                     // for STL hash_map container
#endif //  __GNUC__ >=3
#include "namespace_trans.h"
#include "libiberty/libiberty.h"	// for lbasename()
#include <time.h>                       // for time()

#include "linker.h"                     // std. linker's headers
#include "process.h"                    // for tmpdir, etc.
#include "main.h"                       // for arg_vectors
#include "ipc_weak.h"

#include "defs.h"                       // std. mongoose definitions
#include "errors.h"                     // for ErrMsg
#include "erglob.h"                     // error code
#include "glob.h"                       // for Tlog_File_Name
#include "tracing.h"                    // for Set_Trace_File
#include "cxx_memory.h"                 // for CXX_NEW
#include "opcode.h"                     // needed by wn_core.h
#include "wn_core.h"                    // needed by ir_bread.h
#include "pu_info.h"                    // needed by ir_bread.h
#include "ir_bread.h"                   // for WN_get_section_base ()

#include "dwarf_DST_mem.h"              // needed by ipc_file.h
#include "ipc_file.h"                   // for IP_FILE_HDR
#include "ipa_option.h"                 // ipa option flags
#include "ipc_link.h"                   // for ipa_link_line_argv

#if defined(TARG_ST) && defined(__CYGWIN__)
#include <signal.h>
#endif

#ifndef TARG_ST
// [CL] by default, no longer rely on make: launch subcommands
// directly, this makes the MingWin port easier.
#define USE_MAKE
#endif

#ifndef USE_MAKE
#include "W_wait.h"
#include "W_signal.h"
#include "sys_process.h"
#include <dirent.h>
#define LOGFILE "/var/log/messages"
#endif

#ifndef TARG_ST
#pragma weak tos_string
#pragma weak outfilename
#endif


#if defined(__linux__) || defined(__sun__) || defined(__CYGWIN__) || defined(__MINGW32__)
#define _USE_GNU_MAKE_
#endif

#ifdef _USE_GNU_MAKE_
#define TARGET_DELIMITER " : "
#else
#define TARGET_DELIMITER " ! "
#endif

// To create an executable, we have to 
// -- Compile the symtab file to a .o file (an elf symtab)
// -- Compile the symtab to a .G file
// -- Using the .G file, compile each of the regular whirl files
//    (each file that contains a pu) to an elf .o file
// -- Link all of the .o files.

// For each of the regular whirl files, we keep track of 
// -- its name
// -- the command line we'll use to compile it
// -- a list of zero of more lines that will be added to the makefile
//    as comments.  Normally these will be the pu's contained in the file.

// How to use these functions:
// (1) Initialize with ipa_compile_init.
// (2) Call ipacom_process_symtab, where the argument is the symtab file
//     name.  (e.g. "symtab.I")
// (3) For each other whirl file:
//     (a) call ipacom_process_file
//     (b) call ipacom_add_comment zero or more times, using the 
//         index that ipacom_process_file returned.
// (4) call ipacom_doit.  ipacom_doit never returns.


// --------------------------------------------------------------------------
// File-local variables

// makefile_name is a full pathname, as is outfiles_fullpath, 
// all of the others are basename only.

#ifdef USE_MAKE
static char* makefile_name = 0;         // name of the makefile
static FILE* makefile = 0; 
#else
static char* current_dir = NULL;
#endif

static std::vector<const char*>* infiles = 0;
static std::vector<const char*>* outfiles = 0;
static std::vector<const char*>* outfiles_fullpath = 0;
static std::vector<const char*>* commands = 0;
static std::vector<UINT32>* ProMP_Idx = 0;
#ifdef USE_MAKE
static std::vector<std::vector<const char*> >* comments = 0;
#endif

// Name of the symtab file as written by the ipa.  (e.g. symtab.I)
static char input_symtab_name[PATH_MAX] = "";

// Name of the symtab file that will be used to compile other whirl
// files. (e.g. symtab.G)
static char whirl_symtab_name[PATH_MAX] = "";

// Name of the elf symtab file. (e.g. symtab.o)
static char elf_symtab_name[PATH_MAX] = "";

// Command line for producing the whirl and elf symtab files.  
static const char* symtab_command_line = 0;
static const char* symtab_extra_args = 0;


// Map from short forms of command names (e.g. "cc") to full 
// forms (e.g. "/usr/bin/cc").

namespace {
  struct eqstr {
    bool operator()(const char* s1, const char* s2) const
      { return strcmp(s1, s2) == 0; }
  };
  typedef __GNUEXT::hash_map<const char*, const char*, __GNUEXT::hash<const char*>, eqstr>
          COMMAND_MAP_TYPE;
}

static COMMAND_MAP_TYPE* command_map;

// --------------------------------------------------------------------------

// Overview of what happens post-ipa.  We create a makefile in the
// tmpdir.  We spawn a shell to invoke make.  Make will compile each
// of the .I files in tmpdir, and will then do the final link in the
// current working directory.  After make terminates, the shell
// deletes all of the in the tmp_file_list, including the makefile
// itself and the shell script itself, moves all of the other files in
// tmpdir into the current working directory, and delete the tmpdir.
// Upon interrupt (using sh's builtin trap command), it does the same
// thing.

static const char* get_extra_args(const char* ipaa_filename);
static const char* get_extra_symtab_args(const ARGV&);
#ifdef USE_MAKE
static void exec_smake(char* cmdfile_name);
#endif

/*
  This is here because the gnu basename() doesn't strip
  off multiple slashes.
  [CG]: Use libiberty lbasename
*/
static const char*ipa_basename(char *name){
#ifdef TARG_ST
    return lbasename(name);
#else
    const char *bname = basename(name);
    
    while (*bname == '/')
    	bname++;
    return bname;
#endif
}

static const char* abi()
{
  unsigned int abi_flag = ld_ipa_opt[LD_IPA_TARGOS].flag;
  Is_True(abi_flag < TOS_MAX, ("Invalid abi flag %d\n", (int) abi_flag));

#ifdef _TARG_MIPS
  switch(abi_flag) {
  case TOS_MIPS_O32:
    return "-o32";
  case TOS_MIPS_R32:
    return "-r32";
  case TOS_MIPS_N32:
    return "-n32";
  case TOS_MIPS_64:
    return "-64";
  default:
    Fail_FmtAssertion ("ipa: invalid abi flag");
  }
#endif

#ifdef TARG_IA64
    return "-i64";
#endif

#ifdef _TARG_IA32
    return "-i32";
#endif

#ifdef TARG_ST // [CL]
    return "";
#endif

  return "-n32";                // This line is never reached.
}

namespace {

// Returns true if path refers to an ordinary file.
bool file_exists(const char* path)
{
  if (!path || strlen(path) == 0)
    return false;

  struct stat buf;
  return stat(path, &buf) == 0 && S_ISREG(buf.st_mode);
}

} // Close unnamed namespace

extern "C" void
ipa_compile_init ()
{ 
  Is_True(tmpdir, ("no IPA temp. directory"));

#ifdef USE_MAKE
  Is_True(infiles == 0 && outfiles == 0 && commands == 0 && comments == 0
          && makefile_name == 0 && makefile == 0 && command_map == 0,
          ("ipa_compile_init already initialized"));
#else
  Is_True(infiles == 0 && outfiles == 0 && command_map == 0,
          ("ipa_compile_init already initialized"));
#endif

  infiles           = CXX_NEW (std::vector<const char*>,          Malloc_Mem_Pool);
  outfiles          = CXX_NEW (std::vector<const char*>,          Malloc_Mem_Pool);
  outfiles_fullpath = CXX_NEW (std::vector<const char*>,          Malloc_Mem_Pool);
  commands          = CXX_NEW (std::vector<const char*>,          Malloc_Mem_Pool);
#ifdef USE_MAKE
  comments          = CXX_NEW (std::vector<std::vector<const char*> >, Malloc_Mem_Pool);

  if (infiles == 0 || outfiles == 0 || outfiles_fullpath == 0 ||
      commands == 0 || comments == 0)
#else
  if (infiles == 0 || outfiles == 0 || outfiles_fullpath == 0 ||
      commands == 0)
#endif
    ErrMsg (EC_No_Mem, "ipa_compile_init");

  if (ProMP_Listing)
      ProMP_Idx = CXX_NEW (std::vector<UINT32>, Malloc_Mem_Pool);

#ifdef USE_MAKE
  char name_buffer[256];
  sprintf(name_buffer, "makefile.ipa%ld", (long) getpid());

  makefile_name = create_unique_file(name_buffer, 0);
  add_to_tmp_file_list (makefile_name);

  makefile = fopen(makefile_name, "w");
  if (makefile == 0)
    ErrMsg (EC_Ipa_Open, makefile_name, sys_errlist[errno]);
  chmod(makefile_name, 0644);
#endif

  command_map = CXX_NEW(COMMAND_MAP_TYPE, Malloc_Mem_Pool);
  if (command_map == 0)
    ErrMsg (EC_No_Mem, "ipa_compile_init");

  char* toolroot = getenv("TOOLROOT");

#ifdef TARG_IA64

  static char* smake_base = "/usr/bin/make";
  static char* tmp_cc_name_base = "/usr/ia64-sgi-linux/bin/sgicc";
  static char* cc_name_base = "/usr/bin/sgicc";

  if (file_exists(tmp_cc_name_base))
      cc_name_base = tmp_cc_name_base;

#define MAKE_STRING "make"

#else

#ifdef TARG_ST
#ifndef _USE_GNU_MAKE_
  static char* smake_base = "make";
  static char* tmp_cc_name_base = "cc";
  static char* cc_name_base = "cc";
#define MAKE_STRING "make"
#else
  static char* smake_base = "gmake";
  static char* tmp_cc_name_base = "cc";
  static char* cc_name_base = "cc";
#define MAKE_STRING "gmake"
#endif
#else

  static char* smake_base = "/usr/sbin/smake";
  static char* tmp_cc_name_base = "/usr/bin/cc";
  static char* cc_name_base = "/usr/bin/cc";
#define MAKE_STRING "smake"

#endif // [CL]
#endif

  if (toolroot) {
      static char* new_cc_name_base = concat_names(toolroot, tmp_cc_name_base);
      if (file_exists(new_cc_name_base))
	  (*command_map)["cc"] = new_cc_name_base;
      else 
	  (*command_map)["cc"] = cc_name_base;
  }
  else 
      (*command_map)["cc"]    = cc_name_base;

#ifdef TARG_ST // [CL]
  // If the compile command is supplied through an intermediate file
  // (generated by the compiler driver), use it
  if (Ipa_Compiler_Command_Line) {
    struct stat buf;
    if (!stat(Ipa_Compiler_Command_Line, &buf)) {
      if (buf.st_size > 0) {
	char* compile_cmd = static_cast<char*>(malloc(buf.st_size+1));
	if (!compile_cmd) ErrMsg(EC_No_Mem, "get_command_line");

	FILE* compiler = fopen(Ipa_Compiler_Command_Line, "r");
	// Read only the first line (the 2nd one is cwd)
	if (fgets(compile_cmd, buf.st_size+1, compiler) != compile_cmd) {
	  ErrMsg(EC_Inv_Ipa, Ipa_Compiler_Command_Line);
	}
	fclose(compiler);
	(*command_map)["cc"] = compile_cmd;
	(*command_map)["compiler"] = compile_cmd;
      }
    }
  }
#endif

#ifdef USE_MAKE
  // For smake we first try $TOOLROOT/usr/sbin/smake.  If that doesn't
  // exist then we try /usr/sbin/smake, and finally if *that* doesn't
  // exist we just use smake and hope that the user's search path 
  // contains something sensible.
  static const char* smake_name = 0;
  {
    if (toolroot != 0) {
      const char* tmp = concat_names(toolroot, smake_base);
      if (file_exists(tmp))
        smake_name = tmp;
    }

    if (smake_name == 0) {
      if (file_exists(smake_base))
        smake_name = smake_base;
      else
        smake_name = MAKE_STRING;
    }
  }

  (*command_map)[MAKE_STRING] = smake_name;
#endif

#ifdef TODO
  if (IPA_Enable_Cord) {
    call_graph_file_name = create_unique_file ("ipa_cg", 0);
    add_to_tmp_file_list (call_graph_file_name);
    Call_graph_file = FOPEN (call_graph_file_name, "w");
    if (Call_graph_file == 0)
#ifdef TARG_IA64
      {
      perror(call_graph_file_name);
      exit(1);
      }
#else
      msg (ER_FATAL, ERN_IO_FATAL, call_graph_file_name,
           sys_errlist[errno]); 
#endif
  }
#else
  DevWarn ("TODO: support ipa-cord");
#endif
} // ipa_compile_init


// Generate a command line to compile an ordinary whirl file into an object
// file.
static void
get_command_line (const IP_FILE_HDR& hdr, ARGV& argv, const char* inpath,
                  const char* outpath) 
{
  char* base_addr = (char*)
    WN_get_section_base (IP_FILE_HDR_input_map_addr (hdr), WT_COMP_FLAGS);

  if (base_addr == (char*) -1)
    ErrMsg (EC_IR_Scn_Read, "command line", IP_FILE_HDR_file_name (hdr));

  Elf64_Word argc = *((Elf64_Word *) base_addr);
  Elf64_Word* args = (Elf64_Word *) (base_addr + sizeof(Elf64_Word));

  // args[0] is the command, so we need to treat it specially.  If
  // TOOLROOT is set, and if args[0] isn't already an absolute pathname,
  // we need to construct an absolute pathname using TOOLROOT.

  Is_True(command_map != 0
            && command_map->find("cc") != command_map->end()
            && (*command_map)["cc"] != 0
            && strlen((*command_map)["cc"]) != 0,
          ("Full pathname for cc not set up"));

  if (argc > 0) {
    char* command = base_addr + args[0];

    // Look up the command name in the map; we may need to turn into into
    // a full pathname.
    if (command_map->find(command) == command_map->end()) {
      char* toolroot = getenv("TOOLROOT");
      if (toolroot == 0 || strchr(command, '/') != 0) 
        (*command_map)[command] = command;
      else {
        int len = strlen(toolroot) + strlen(command) + 116;
        char* buf = static_cast<char*>(malloc(len));
        if (!buf)
          ErrMsg (EC_No_Mem, "get_command_line");
        strcpy(buf, toolroot);
#ifdef TARG_IA64
        strcat(buf, "/usr/ia64-sgi-linux/bin/");
        strcat(buf, command);
        if (!file_exists(buf)) {
	    bzero(buf, strlen(buf));
	    strcpy(buf, toolroot);
            strcat(buf, "/usr/bin/");
            strcat(buf, command);
	}
        (*command_map)[command] = buf;

#else
        strcat(buf, "/usr/bin/");
        strcat(buf, command);
        (*command_map)[command] = buf;
#endif
      }
    }

    Is_True(command_map->find(command) != command_map->end()
            && (*command_map)[command] != 0
            && strlen((*command_map)[command]) != 0,
            ("Full pathname for %s not found in command map", command));
#ifdef TARG_ST
    if (command_map->find("compiler") != command_map->end()
	&& (*command_map)["compiler"] != 0
	&& strlen((*command_map)["compiler"]) != 0) {
      argv.push_back((*command_map)["compiler"]);
    } else {
#endif
    argv.push_back((*command_map)[command]);

    for (INT i = 1; i < argc; ++i) {
      argv.push_back (base_addr + args[i]);
    }
#ifdef TARG_ST
    }
#endif
  }
  else {
    argv.push_back ((*command_map)["cc"]);
    argv.push_back ("-c");
  }

  argv.push_back(abi());

  argv.push_back (inpath);
  argv.push_back ("-o");
  argv.push_back (outpath);
  argv.push_back ("-c");
    
} // get_command_line


// temp. kludge:  ipacom_process_file should really take
// const IP_FILE_HDR& as argument instead of const PU_Info *
#include "ipc_symtab_merge.h"
static const IP_FILE_HDR&
get_ip_file_hdr (const PU_Info *pu)
{
  ST_IDX st_idx = PU_Info_proc_sym (pu);
  PU_IDX pu_idx = ST_pu (St_Table[st_idx]);
  return *AUX_PU_file_hdr (Aux_Pu_Table[pu_idx]);
}

extern "C" void
ipacom_process_symtab (char* symtab_file)
{

#ifdef USE_MAKE
  Is_True(infiles != 0 && outfiles != 0 && outfiles_fullpath != 0 &&
          commands != 0 && comments != 0,
          ("ipacom_process_symtab: ipacom not yet initialized"));
#else
  Is_True(infiles != 0 && outfiles != 0 && outfiles_fullpath != 0 &&
          commands != 0,
          ("ipacom_process_symtab: ipacom not yet initialized"));
#endif

  Is_True(strlen(input_symtab_name) == 0 &&
          strlen(whirl_symtab_name) == 0 &&
          strlen(elf_symtab_name) == 0 &&
          symtab_command_line == 0,
          ("ipacom_process_symtab: symtab already initialized"));

  char* output_file = create_unique_file (symtab_file, 'o');
  add_to_tmp_file_list (output_file);
#ifdef _USE_GNU_MAKE_
  unlink (output_file);
#endif

  const char* input_base = ipa_basename(symtab_file);
  const char* output_base = ipa_basename(output_file);

  // Save the three symtab file names in global variables.
  strcpy(input_symtab_name, input_base);
  strcpy(elf_symtab_name,   output_base);
  strcpy(whirl_symtab_name, output_base);
  whirl_symtab_name[strlen(whirl_symtab_name) - 1] = 'G';

  // Generate a command line to create the .G and .o files.
  char buf[3*PATH_MAX + 64];

  Is_True(command_map != 0
            && command_map->find("cc") != command_map->end()
            && (*command_map)["cc"] != 0
            && strlen((*command_map)["cc"]) != 0,
          ("Full pathname for cc not set up"));

#if defined(TARG_ST) && defined(Is_True_On)
  // [CL] do not embed "cc" name yet. Update it later,
  // when it has been determined by other input files
  sprintf(buf, " -c %s %s -o %s -TENV:emit_global_data=%s %s %s",
            abi(),
            input_symtab_name,
            elf_symtab_name,
            whirl_symtab_name,
            IPA_Enable_AutoGnum?"-Gspace 0":"",
  			ld_ipa_opt[LD_IPA_SHOW_ONLY].flag?"-Wb,-ttMSC:0x40":"");
#else
  sprintf(buf, "%s -c %s %s -o %s -TENV:emit_global_data=%s %s",
            (*command_map)["cc"],
            abi(),
            input_symtab_name,
            elf_symtab_name,
            whirl_symtab_name,
            IPA_Enable_AutoGnum?"-Gspace 0":"");
#endif


  char* cmd = static_cast<char*>(malloc(strlen(buf) + 1));
  if (!cmd)
    ErrMsg (EC_No_Mem, "ipacom_process_symtab");

  strcpy(cmd, buf);
  symtab_command_line = cmd;

  Is_True(strlen(input_symtab_name) != 0 &&
          strlen(whirl_symtab_name) != 0 &&
          strlen(elf_symtab_name) != 0 &&
          symtab_command_line != 0,
          ("ipacom_process_symtab: initialization failed"));

} // ipacom_process_symtab

// The return value is the index of this file in the vectors.
extern "C" 
size_t ipacom_process_file (char* input_file,
                            const PU_Info* pu, UINT32 ProMP_id)
{
#ifdef USE_MAKE
  Is_True(infiles != 0 && outfiles_fullpath != 0 && commands != 0 &&
          comments != 0,
          ("ipacom_process_file: ipacom not initialized"));
#else
  Is_True(infiles != 0 && outfiles_fullpath != 0 && commands != 0,
          ("ipacom_process_file: ipacom not initialized"));
#endif

  Is_True(strlen(input_symtab_name) != 0 &&
          strlen(whirl_symtab_name) != 0 &&
          strlen(elf_symtab_name) != 0 &&
          symtab_command_line != 0,
          ("ipacom_process_file: symtab not initialized"));

  if (ProMP_Listing) {
      Is_True (ProMP_Idx != 0,
	       ("ipacom_process_file: ipacom not initialized"));
      ProMP_Idx->push_back (ProMP_id);
  }

  char* output_file = create_unique_file (input_file, 'o');

  add_to_tmp_file_list (output_file);

  const char* input_base = ipa_basename (input_file);
  const char* output_base = ipa_basename (output_file);

  infiles->push_back(input_base);
  outfiles->push_back(output_base);
  outfiles_fullpath->push_back(output_file);

  // Assemble the command line.

  ARGV argv;                          // vector<const char*>
  get_command_line (get_ip_file_hdr (pu), argv, input_base, output_base);

  char* str = (char*) malloc(2 * PATH_MAX + 64);
  sprintf(str, "-TENV:ipa_ident=%d -TENV:read_global_data=%s %s",
          (int)time(0),
          whirl_symtab_name,
          IPA_Enable_AutoGnum?"-Gspace 0":"");


  argv.push_back(str);

  if (ProMP_Listing) {
    char* str = static_cast<char*>(malloc(64));
    sprintf(str, "-PROMP:=ON -PROMP:next_id=%lu", (unsigned long) ProMP_id);
    argv.push_back(str);
  }
    
  //char* gspace = (char*) malloc(2 * PATH_MAX + 64);
  //strcpy(gspace, "-Gspace 0");
  //argv.push_back(gspace);

#if defined(TARG_ST) && defined(Is_True_On)
  if (ld_ipa_opt[LD_IPA_SHOW_ONLY].flag) {
    argv.push_back("-Wb,-ttMSC:0x40");
  }
#endif
  
#ifdef TODO
  if (gspace_size) {
    WRITE_STRING("-Gspace", argv->argv[i]);
    sprintf(str, "%d", gspace_size);
    WRITE_STRING(str, argv->argv[++i]);
  }
#else
  static bool reported = false;
  if (!reported) {
    reported = true;
    DevWarn ("TODO: implement gspace_size command file");
  }
  if (IPA_Enable_Array_Sections)
    argv.push_back("-LNO:ipa");
#endif

  // Piece the command line together and push it onto the list.
  size_t cmdline_length = 0;
  ARGV::const_iterator i;

  for (i = argv.begin(); i != argv.end(); ++i)
    cmdline_length += strlen(*i) + 1;

  char* cmdline = static_cast<char*>(malloc(cmdline_length + 1));
  if (!cmdline)
    ErrMsg (EC_No_Mem, "ipacom_process_file");    

  cmdline[0] = '\0';
    
  for (i = argv.begin(); i != argv.end(); ++i) {
    strcat(cmdline, *i);
    strcat(cmdline, " ");
  }

  commands->push_back(cmdline);
    
#ifdef USE_MAKE
  // Add an empty vector for this file's comments.
  // [HK]
#if __GNUC__ >= 3
  comments->push_back(std::vector<const char*>());
#else
  comments->push_back();
#endif // __GNUC__ >= 3

  Is_True (infiles->size() > 0 &&
           infiles->size() == outfiles->size() &&
           infiles->size() == outfiles_fullpath->size() &&
           infiles->size() == commands->size() &&
           infiles->size() == comments->size(),
           ("ipacom_process_file: inconsistent vector sizes"));
#else
  Is_True (infiles->size() > 0 &&
           infiles->size() == outfiles->size() &&
           infiles->size() == outfiles_fullpath->size() &&
           infiles->size() == commands->size(),
           ("ipacom_process_file: inconsistent vector sizes"));
#endif

  // Set up extra args for compiling symtab, if necessary.
  if (!symtab_extra_args)
    symtab_extra_args = get_extra_symtab_args(argv);

  return infiles->size() - 1;

} // ipacom_process_file

// Each file has a list of zero or more comments that will appear in the
// makefile.  (Usually, each comment will be the name of a pu.)  
// This function adds a comment to the n'th file's list.
extern "C"
void ipacom_add_comment(size_t n, const char* comment)
{
#ifdef USE_MAKE
  Is_True(infiles != 0 && outfiles != 0 && outfiles_fullpath != 0 &&
          commands != 0 && comments != 0,
          ("ipacom_add_comment: ipacom not initialized"));

  Is_True(comments->size() >= n + 1,
          ("ipacom_add_comment: invalid index %ld, max is %ld",
           n, comments->size()));

  Is_True(comment != 0, ("ipacom_add_comment: argument is a null pointer"));

  char* tmp = static_cast<char*>(malloc(strlen(comment) + 1));
  if (!tmp)
    ErrMsg (EC_No_Mem, "ipacom_add_commend");
  strcpy(tmp, comment);

  (*comments)[n].push_back(tmp);
#endif
}

namespace {

char* ipc_copy_of (char *str)
{
  register int len;
  register char *p;

  len = strlen(str) + 1;
  p = (char *) MALLOC (len);
  MALLOC_ASSERT (p);
  BCOPY (str, p, len);
  return p;
} /* ipc_copy_of */

#ifdef TARG_ST
  // [CL] Handle the case where no input file
  // contained Whirl
  // In this case, outfiles is empty and we
  // have to create a dummy dependency to
  // force 'true' ld to be called again
bool has_dummy2 = FALSE;
#endif

#ifdef USE_MAKE
void print_all_outfiles(const char* dirname)
{
  for (std::vector<const char*>::iterator i = outfiles->begin();
       i != outfiles->end();
       ++i)
    fprintf(makefile, "%s%s/%s \\\n", "   ", dirname, *i);
  
  if (strlen(elf_symtab_name) != 0)
    fprintf(makefile, "%s%s/%s \n", "   ", dirname, elf_symtab_name);
#ifdef TARG_ST
  else
    if (outfiles->begin() == outfiles->end()) {
      has_dummy2 = TRUE;
      fprintf(makefile, "%s%s/dummy2\n", "   ", dirname);
    } else {
      fprintf(makefile, "\n");
    }
#endif
}
#endif

} // Close unnamed namespace

#ifndef USE_MAKE
	  // [CL] convert string arguments into argv[]
void string2argv(const char* string, ARGV& argv)
{
  char *mystring = CXX_NEW_ARRAY(char, strlen(string)+1, Malloc_Mem_Pool);

  char* curptr = mystring;
  char* ptr = curptr;
  strcpy(ptr, string);

  // Simple handling of quote chars: based on the assumption that
  // quote chars have been added by the driver, simply remove them
  // now, and accept spaces within the current string.

#define QUOTE_CHAR(c)				\
  ( (c) == '"' ||				\
    (c) == '\'' ||				\
    (c) == '`' )
  int quote = 0;
  int isspace;
  
  while(*ptr) {
    isspace = 0;
    curptr = ptr;
    while(*curptr) {
      if (QUOTE_CHAR(*curptr)) {
	if (quote == 0) {
	  quote++;
	} else {
	  quote--;
	}
	// remove quoting char
	strcpy(curptr, curptr+1);
	continue;
      }
      if ((*curptr) == ' ') {
	if (quote == 0) {
	  *curptr = '\0';
	  isspace = 1;
	  break;
	}
      }
      curptr++;
    }
    if (strlen(ptr) > 0) {
      argv.push_back(ptr);
    }
    if (isspace) {
      ptr = curptr + 1;
    } else {
      ptr = curptr;
    }
  }
}

// [CL] code cloned from the driver
#include <cmplrs/rcodes.h>

static void error(string format, ...)
{
	va_list args;
	va_start (args, format);
	fprintf(stderr, "ERROR:  ");
	vfprintf(stderr, format, args);
	fprintf(stderr, "\n");
	va_end (args);
}

static void internal_error (string format, ...)
{
	va_list args;
	va_start (args, format);
	fprintf(stderr, "INTERNAL ERROR:  ");
	vfprintf(stderr, format, args);
	fprintf(stderr, "\n");
	va_end (args);
}

static void remove_file(const char* name)
{
  int status;

  status = unlink(name);
  if (status != 0 && errno != ENOENT) {
    internal_error("cannot unlink temp file %s", name);
  }
}

// Concatenate file 'srcname' into 'dstname'
#define MAX_RBUF_SIZE 1*1024*1024
static void concat(const char* srcname, const char* dstname)
{
  void *rbuf = NULL;
  size_t size_buf = 0;
  long cur_size = 0;
  struct stat buf;
  int elem_size;
  int dst_fd, src_fd;
  int mode=0666;

  if (stat(srcname, &buf)) {
    perror("error generating log file");
    return;
  }

  elem_size = buf.st_size;

  if (elem_size > MAX_RBUF_SIZE) {
      rbuf = (void *) malloc(MAX_RBUF_SIZE);
      if (rbuf == NULL) {
	perror("error generating log file");
	return;
      }
      size_buf = MAX_RBUF_SIZE;
  } else {
      rbuf = (void *) malloc(elem_size);
      if (rbuf == NULL) {
	perror("error generating log file");
	return;
      }
      size_buf = elem_size;
  }

  src_fd = open (srcname, O_RDONLY, mode);
  if (src_fd == -1) {
    perror("error generating log file");
    return;
  }

  dst_fd = open (dstname, O_RDWR|O_CREAT|O_APPEND, mode);
  if (dst_fd == -1) {
    perror("error generating log file");
    close(src_fd);
    free(rbuf);
    return;
  }

  cur_size = elem_size;
  while (cur_size > 0) {
      ssize_t ret;

      ret = read(src_fd, rbuf, size_buf);
      if (ret > 0) {
	  ssize_t w_ret;

	  w_ret = write(dst_fd, rbuf , ret);
	  if (w_ret == -1) {
	    perror("error generating log file");
	    close(dst_fd);
	    close(src_fd);
	    free(rbuf);
	    return;
	  }
	  cur_size -= ret;
      } else {
	perror("error generating log file");
	close(dst_fd);
	close(src_fd);
	free(rbuf);
	return;
      }
  }
  
  free(rbuf);
  close(dst_fd);
  close(src_fd);
}

static void cleanup()
{
  // Concatenate all *.t files into 1 single trace file. Same for *.tlog
  DIR* dirp = opendir(tmpdir);
  bool removed_t_file = false;
  bool removed_tlog_file = false;

  if (dirp) {
    struct dirent* direntp;

    while ((direntp = readdir(dirp)) != NULL) {
      unsigned int len = strlen(direntp->d_name);
      // check if *.t file
      if (len > 1){
	if (strcmp(&direntp->d_name[len-2], ".t") == 0) {
	  // Concat .t file
	  char* tname = (char*)malloc(strlen(outfilename)+1+strlen(".t"));
	  sprintf(tname, "%s.t", outfilename);

	  char* srcname = (char*)malloc(strlen(tmpdir)+strlen(direntp->d_name)+1+1);
	  sprintf(srcname, "%s/%s", tmpdir, direntp->d_name);

	  // First, remove any existing file if we are about to concat new ones
	  if (!removed_t_file) {
	    remove_file(tname);
	    removed_t_file = true;
	  }

	  concat(srcname, tname);

	  free(tname);
	  free(srcname);

	} else if (strcmp(&direntp->d_name[len-5], ".tlog") == 0) {
	  // Concat .tlog file
	  char* tlogname = (char*)malloc(strlen(outfilename)+1+strlen(".tlog"));
	  sprintf(tlogname, "%s.tlog", outfilename);

	  char* srcname = (char*)malloc(strlen(tmpdir)+strlen(direntp->d_name)+1+1);
	  sprintf(srcname, "%s/%s", tmpdir, direntp->d_name);

	  // First, remove any existing file if we are about to concat new ones
	  if (!removed_tlog_file) {
	    remove_file(tlogname);
	    removed_tlog_file = true;
	  }

	  concat(srcname, tlogname);

	  free(tlogname);
	  free(srcname);
	}
      }
    }
    closedir(dirp);
  }

  Unmap_All();

  if (ld_ipa_opt[LD_IPA_KEEP_TEMPS].flag) {
    return;
  }

  chdir(tmpdir);

  // Remove each temporary file that we know about.
  std::vector<const char*>::iterator i;
  for (i = infiles->begin(); i != infiles->end(); ++i) {
    remove_file(*i);
  }

  for (i = outfiles->begin(); i != outfiles->end(); ++i) {
    remove_file(*i);
  }

  if (infiles->size() > 0) {
    remove_file(input_symtab_name);
    remove_file(whirl_symtab_name);
    remove_file(elf_symtab_name);
  }

  // [CL] go out of tmpdir before removing it
  chdir(current_dir);

  // [CL] remove all files in tmpdir (ie archive members compiled with
  // -ipa, that have been extracted by the linker, and not removed
  // yet)
  dirp = opendir(tmpdir);
  if (dirp) {
    struct dirent* direntp;

    while ((direntp = readdir(dirp)) != NULL) {
      if (direntp->d_name[0] == '.') {
	if ( ((direntp->d_name[1] == '.')
	      && (direntp->d_name[2] == '\0'))
	     || (direntp->d_name[1] == '\0')
	     ) {
	  continue;
	}
      }
	     
      char* srcname = (char*)malloc(strlen(tmpdir)+strlen(direntp->d_name)+2);
      sprintf(srcname, "%s/%s", tmpdir, direntp->d_name);
      remove_file(srcname);
      free(srcname);
    }
    closedir(dirp);
  }

  int res = rmdir(tmpdir);
  if (res) {
    fprintf(stderr, "Couldn't remove %s: ", tmpdir);
    perror("");
  }
}

// [CL] code cloned from the driver
static int run_program(ARGV& myargv)
{
  int status = -1;
  int pid, waitstatus, termsig;

  const char* symtab_cmd_argv[myargv.size()];
  char *errmsg_fmt, *errmsg_arg;

  int i = 0;
  for (ARGV::const_iterator it = myargv.begin();
       it != myargv.end();
       ++it, i++) {
    symtab_cmd_argv[i] = *it;
#ifdef TARG_ST
    if (ld_ipa_opt[LD_IPA_SHOW].flag || ld_ipa_opt[LD_IPA_SHOW_ONLY].flag) {
#else
    if (ld_ipa_opt[LD_IPA_SHOW].flag) {
#endif
      fprintf(stderr, "%s ", *it);
    }
  }
  symtab_cmd_argv[i] = NULL;
#ifdef TARG_ST
    if (ld_ipa_opt[LD_IPA_SHOW].flag || ld_ipa_opt[LD_IPA_SHOW_ONLY].flag) {
#else
    if (ld_ipa_opt[LD_IPA_SHOW].flag) {
#endif
    fprintf(stderr, "\n");
  }

  const char* program = symtab_cmd_argv[0];

  pid = sys_pexecute(symtab_cmd_argv[0], symtab_cmd_argv, (const char **)&errmsg_fmt,
			 (const char **)&errmsg_arg, SYS_P_STOP_ON_EXIT, NULL, NULL, NULL);

  /* Check return of pexecute. */
  if (pid < 0) {
    error(errmsg_fmt, errmsg_arg);
    cleanup ();
    exit (RC_SYSTEM_ERROR);
    /* NOTREACHED */
  }
  
  pid = sys_pwait(pid, &waitstatus, 0);
  if (pid < 0) {
    error("unexpected error waiting for %s", program);
    cleanup();
    exit(RC_SYSTEM_ERROR);
    /* NOTREACHED */
  }
  
  pid = sys_pwait(pid, &waitstatus, SYS_P_TERM);
  if (pid < 0) {
    error("unexpected error waiting for %s", program);
    cleanup();
    exit(RC_SYSTEM_ERROR);
    /* NOTREACHED */
  }
#ifdef WIFSTOPPED
  if (WIFSTOPPED(waitstatus)) {
    termsig = WSTOPSIG(waitstatus);
    error("STOPPED signal %d received by %s", program, termsig);
    cleanup();
    exit(RC_SYSTEM_ERROR);
    /* NOTREACHED */
  } 
#endif
  if (WIFEXITED(waitstatus)) {
    status = WEXITSTATUS(waitstatus);
    /* Will return at end of function with status set. */
  } else if(WIFSIGNALED(waitstatus)){
    termsig = WTERMSIG(waitstatus);
    error("%s died due to signal %d", program, termsig);
#ifdef WCOREFLAG
    if(waitstatus & WCOREFLAG) {
      error("core dumped");
    }
#endif
#ifdef SIGKILL
    if (termsig == SIGKILL) {
      error("Probably caused by running out of swap space -- check %s", LOGFILE);
    }
#endif
    cleanup();
    exit(RC_SYSTEM_ERROR);
    /* NOTREACHED */
  } else {
    /* cannot happen, I think! */
    internal_error("unexpected status returned by %s", program);
    cleanup();
    exit(RC_SYSTEM_ERROR);
    /* NOTREACHED */
  }
  return status;
}
#endif

extern "C"
void ipacom_doit (const char* ipaa_filename)
{
#ifdef USE_MAKE
  Is_True(infiles != 0 && outfiles != 0 && outfiles_fullpath != 0 &&
          commands != 0 && comments != 0 && makefile != 0,
          ("ipacom_doit: ipacom not yet initialized"));
#else
  Is_True(infiles != 0 && outfiles != 0 && outfiles_fullpath != 0 &&
          commands != 0,
          ("ipacom_doit: ipacom not yet initialized"));
#endif

#ifdef USE_MAKE
  Is_True(infiles->size() == outfiles->size() &&
          infiles->size() == outfiles_fullpath->size() &&
          infiles->size() == commands->size() &&
          infiles->size() == comments->size(),
          ("ipacom_doit: vectors are inconsistent"));
#else
  Is_True(infiles->size() == outfiles->size() &&
          infiles->size() == outfiles_fullpath->size() &&
          infiles->size() == commands->size(),
          ("ipacom_doit: vectors are inconsistent"));
#endif


  if (infiles->size() > 0) {
    Is_True(strlen(input_symtab_name) != 0 &&
            strlen(whirl_symtab_name) != 0 &&
            strlen(elf_symtab_name) != 0 &&
            symtab_command_line != 0,
            ("ipacom_doit: symtab not initialized"));
  }

#ifdef TODO
  if (IPA_Enable_Cord) {
    FCLOSE (Call_graph_file);
    if (IPA_Enable_final_link)
      process_cord_cmd ();
  }
#endif

  // These are used when compiling each .I file.
  const char* extra_args = get_extra_args(ipaa_filename);

  const char* tmpdir_macro_name = "IPA_TMPDIR";
  const char* tmpdir_macro      = "$(IPA_TMPDIR)";
#ifdef USE_MAKE
  fprintf(makefile, "%s = %s\n\n", tmpdir_macro_name, tmpdir);
#endif

  char* link_cmdfile_name = 0;

  // The default target: either the executable, or all of the
  // elf object files.

#ifndef USE_MAKE
    ARGV link_argv;

    // remember current dir
    if ((current_dir = getcwd(NULL, PATH_MAX)) == NULL) {
      internal_error("could not find current working directory");
    }
#endif

  if (IPA_Enable_final_link) {
    // Path (possibly relative to cwd) of the executable we're creating.
#ifdef USE_MAKE
    const char* executable = outfilename;
    const char* executable_macro_name = "IPA_OUTFILENAME";
    const char* executable_macro      = "$(IPA_OUTFILENAME)";

    fprintf(makefile, "%s = %s\n\n", executable_macro_name, executable);
    fprintf(makefile, "default: %s\n\n", executable_macro);
    fprintf(makefile, ".IGNORE: %s\n\n", executable_macro);
    fprintf(makefile, "%s%s\\\n", executable_macro, TARGET_DELIMITER);

    print_all_outfiles(tmpdir_macro);

#if 0
    fputs("\n", makefile);
#endif

    // The final link command is just ld -from <cmdfile>.  Everything else
    // goes into cmdfile.

    // Create a temporary file for cmdfile.
    char cmdfile_buf[256];
    sprintf(cmdfile_buf, "linkopt.%ld", (long) getpid());
    link_cmdfile_name = create_unique_file(cmdfile_buf, 0);
    FILE* cmdfile = fopen(link_cmdfile_name, "w");
    if (cmdfile == 0)
      ErrMsg (EC_Ipa_Open, link_cmdfile_name, sys_errlist[errno]);
    chmod(link_cmdfile_name, 0644);
#endif /* USE_MAKE */

    // Get the link command line.
    const ARGV* link_line = ipa_link_line_argv (outfiles_fullpath, 
    	    	    	    	    	    	tmpdir, 
						elf_symtab_name);
    Is_True(link_line->size() > 1, ("Invalid link line ARGV vector"));

#ifndef USE_MAKE
    link_argv.push_back(link_line->front());
    if (ld_ipa_opt[LD_IPA_RELOCATABLE].flag) {
      link_argv.push_back("-r");
    }
#endif

    // Print all but link_line[0] into cmdfile.
    ARGV::const_iterator i = link_line->begin();
    for (++i; i != link_line->end(); ++i) {
#ifdef TARG_ST
      // [CL] skip IPA-compiled libraries for final link phase
      const char* flag=*i;
      int len=strlen(flag);
      int ipalen=strlen("-ipa");
      if ( (len >= ipalen+2) &&
	   strncmp(flag, "-l", 2)==0 &&
	   strncmp(&flag[len-ipalen], "-ipa", ipalen)==0 ) {
	continue;
      }

      // [CL] do not pass -shared if we are generating a relocatable
      // object
      if (strcmp(flag, "-shared") == 0
	  && ld_ipa_opt[LD_IPA_RELOCATABLE].flag) {
	continue;
      }
#endif
#ifdef USE_MAKE
      fputs(*i, cmdfile);
      fputs(" \n", cmdfile);
#else
      link_argv.push_back(*i);
#endif
    }

#ifdef USE_MAKE
    fputs("\n", cmdfile);
    fclose(cmdfile);
#endif

#ifdef USE_MAKE
    // If we're compiling with -show, make sure we see the link line.
#ifdef TARG_ST
    if (ld_ipa_opt[LD_IPA_SHOW].flag || ld_ipa_opt[LD_IPA_SHOW_ONLY].flag) {
#else
    if (ld_ipa_opt[LD_IPA_SHOW].flag) {
#endif
#ifndef TARG_ST
      fprintf(makefile, "\techo -n %s ' ' ; cat %s\n",
              link_line->front(), link_cmdfile_name);
#else
      // [CL] add -r if requested
      fprintf(makefile, "\techo -n %s %s ' ' ; cat %s\n",
              link_line->front(),
	      ld_ipa_opt[LD_IPA_RELOCATABLE].flag ? "-r" : "",
	      link_cmdfile_name);
#endif
#ifndef _USE_GNU_MAKE_
      fprintf(makefile, "\t...\n");
#endif
    }
#endif

    // Print the final link command into the makefile.
#ifdef TARG_IA64
    fprintf(makefile, "\t%s `cat %s `\n",
            link_line->front(),
            link_cmdfile_name);
#else
#ifdef TARG_ST
#ifdef USE_MAKE
    fprintf(makefile, "\t%s %s `cat %s `\n",
	    //	    (*command_map)["compiler"],
	    link_line->front(),
	      ld_ipa_opt[LD_IPA_RELOCATABLE].flag ? "-r" : "",
            link_cmdfile_name);
#endif
#else
    fprintf(makefile, "\t%s -from %s\n",
            link_line->front(),
            link_cmdfile_name);
#endif /* TARG_ST */
#endif

#ifdef TARG_ST
#ifdef USE_MAKE
    // [CL] Handle the case where no input file contains Whirl
    if (has_dummy2) {
      fprintf(makefile, "\n%s/dummy2" TARGET_DELIMITER "\n",
	      tmpdir_macro);
    }
#endif
#endif

#ifdef USE_MAKE
    //For ProMP we need to run a Perl script after doing the final link.
    if (ProMP_Listing) {
      char* toolroot = getenv("TOOLROOT");
      static char* script_base = "/usr/lib32/cmplrs/pfa_reshuffle";
      const char* script_name = toolroot ? concat_names(toolroot, script_base)
                                         : script_base;
      struct stat dummy;
      if (stat("/bin/perl5", &dummy) == 0 && stat(script_name, &dummy) == 0) {
        fprintf(makefile, "\t/bin/perl5 %s", script_name);
        std::vector<const char*>::const_iterator i = outfiles_fullpath->begin();
        for ( ; i != outfiles_fullpath->end(); ++i)
          fprintf(makefile, " %s", *i);
        fprintf(makefile, "\n");
      }
      else {
        if (stat("/bin/perl5", &dummy) != 0) {
          DevWarn("Can't find perl5 to run the ProMP reshuffle script");
        }
        if (stat(script_name, &dummy) != 0) {
          DevWarn("Can't find the ProMP reshuffle script");
        }
      }

      // For ProMP we also need to concatenate all of the .list files into
      // a file in the same directory as the excecutable.  The file is
      // named <executable>.list
      fprintf(makefile, "\tif [ -f %s.list ] ; then 'rm' -f %s.list ; fi\n",
              executable_macro, executable_macro);
      fprintf(makefile, "\t'cat' %s/*.list > %s.list\n",
              tmpdir_macro, executable_macro);
    }
#endif

    // Do the same thing with .t and .tlog files (if they exist) as
    // with .list files.
    bool tlogs_enabled = Get_Trace(TP_PTRACE1, 0xffffffff) ||
                         Get_Trace(TP_PTRACE2, 0xffffffff);
    bool t_enabled = TFile != stdout;

#ifdef USE_MAKE
    if (tlogs_enabled) {
      fprintf(makefile, "\tif [ -f %s/*.tlog ] ; then ",
              tmpdir);
      fprintf(makefile, "'cat' %s/*.tlog >> %s.tlog ; true ; fi\n",
              tmpdir, executable);
    }
    if (t_enabled) {
      fprintf(makefile, "\tif [ -f %s/*.t ] ; then ",
              tmpdir);
      fprintf(makefile, "'cat' %s/*.t >> %s.t ; true ; fi\n",
              tmpdir, executable);
    }
#endif
  }
  else {
#ifdef USE_MAKE
    fprintf(makefile, "\ndefault: \\\n");      
    print_all_outfiles(tmpdir_macro);
#endif
  }

#ifdef USE_MAKE
  fputs("\n", makefile);
#endif

#ifdef TARG_ST
  // [CL] generate unique identifier for all following compilations
  INT32 ipa_time = time(0);

  // [CL] use updated "cc" name
  char* relativepath = "";
#endif

  // This generates both the .o symtab and the .G symtab.
  if (strlen(elf_symtab_name) != 0) {
    Is_True(strlen(input_symtab_name) != 0 &&
            strlen(whirl_symtab_name) != 0 &&
            symtab_command_line != 0 && strlen(symtab_command_line) != 0,
            ("ipacom_doit: symtab not initialized"));

    if (!symtab_extra_args)
      symtab_extra_args = get_extra_args(0);

#if defined(TARG_IA64) || defined(TARG_ST)

#ifdef USE_MAKE
    fprintf(makefile, "%s/%s" TARGET_DELIMITER "\n",
            tmpdir_macro, "dummy");
#endif

#ifdef TARG_ST
#if defined(__CYGWIN__) || defined(__MINGW32__)

#define IS_DIR_SEPARATOR(c)     ((c) == '/' || (c) == '\\')
/* Note that IS_ABSOLUTE_PATH accepts d:foo as well, although it is
   only semi-absolute.  This is because the users of IS_ABSOLUTE_PATH
   want to know whether to prepend the current working directory to
   a file name, which should not be done with a name like d:foo.  */
/* The 1st character may also be a double quote */
#define IS_ABSOLUTE_PATH(f)     (IS_DIR_SEPARATOR((f)[0]) || \
				 (((f)[0]) && ((f)[1] == ':')) || \
				 (((f)[0] == '"') && ((f)[1]) && ((f)[2] == ':')) \
    )

#else

#define IS_DIR_SEPARATOR(c)     ((c) == '/')
#define IS_ABSOLUTE_PATH(f)     (IS_DIR_SEPARATOR((f)[0]))

#endif

    if (! IS_ABSOLUTE_PATH((*command_map)["cc"])) {
      if ( ((*command_map)["cc"])[0] != '"' ||  // Handle case of path quoted by the driver
	   (! IS_ABSOLUTE_PATH(&((*command_map)["cc"])[1])) ) {
      relativepath = "../";
      }
    }

#ifdef USE_MAKE
#if 0
    fprintf(makefile, "\tcd %s; %s%s %s -TENV:ipa_suffix=%d_%d %s\n\n",
	    tmpdir_macro, relativepath, (*command_map)["cc"],
	    symtab_command_line, ipa_time, getpid(), symtab_extra_args);
#else
    // [CG] Allow user given -TENV:ipa_suffix to override by passing ipa_suffix before 
    // the original command line
    if (Ipa_Label_Suffix[0] == '\0') {
      fprintf(makefile, "\tcd %s; %s%s -TENV:ipa_suffix=%d_%d %s %s\n\n",
	      tmpdir_macro, relativepath, (*command_map)["cc"],
	      ipa_time, getpid(), symtab_command_line, symtab_extra_args);
    } else {
      fprintf(makefile, "\tcd %s; %s%s  %s %s\n\n",
	      tmpdir_macro, relativepath, (*command_map)["cc"],
	      symtab_command_line, symtab_extra_args);
    }
#endif
#endif
#else
    fprintf(makefile, "\tcd %s; %s %s\n\n",
            tmpdir_macro, symtab_command_line, symtab_extra_args);
#endif
      
#ifdef USE_MAKE
    fprintf(makefile, "%s/%s" TARGET_DELIMITER "%s/%s %s/%s\n\n",
            tmpdir_macro, elf_symtab_name,
            tmpdir_macro, input_symtab_name,
            tmpdir_macro, "dummy");

    fprintf(makefile, "%s/%s" TARGET_DELIMITER "%s/%s %s/%s\n\n",
            tmpdir_macro, whirl_symtab_name,
            tmpdir_macro, elf_symtab_name,
            tmpdir_macro, "dummy");
#endif
#endif

#ifdef _TARG_MIPS

    fprintf(makefile, "%s/%s" TARGET_DELIMITER "%s/%s\n",
            tmpdir_macro, elf_symtab_name,
            tmpdir_macro, input_symtab_name);

    fprintf(makefile, "\tcd -P %s; %s %s\n\n",
            tmpdir_macro, symtab_command_line, symtab_extra_args);
      
    fprintf(makefile, "%s/%s" TARGET_DELIMITER "%s/%s\n\n",
            tmpdir_macro, whirl_symtab_name,
            tmpdir_macro, elf_symtab_name);

#endif

#ifndef USE_MAKE
    ARGV myargv;

    char symtab_buf1[strlen( (*command_map)["cc"]) + strlen(relativepath) + 1];
    sprintf(symtab_buf1, "%s%s", relativepath, (*command_map)["cc"]);
    string2argv(symtab_buf1, myargv);

#if 0
    string2argv(symtab_command_line, myargv);

    char symtab_buf2[100];
    sprintf(symtab_buf2, "-TENV:ipa_suffix=%d_%d", ipa_time, getpid());
    myargv.push_back(symtab_buf2);
#else
    if (Ipa_Label_Suffix[0] == '\0') {
      char symtab_buf2[100];
      sprintf(symtab_buf2, "-TENV:ipa_suffix=%d_%d", ipa_time, getpid());
      myargv.push_back(symtab_buf2);
    }

    string2argv(symtab_command_line, myargv);
#endif

    string2argv(symtab_extra_args, myargv);

#ifdef TARG_ST
    if (ld_ipa_opt[LD_IPA_SHOW].flag || ld_ipa_opt[LD_IPA_SHOW_ONLY].flag) {
#else
    if (ld_ipa_opt[LD_IPA_SHOW].flag) {
#endif
      fprintf(stderr, "cd %s\n", tmpdir);
    }
    chdir(tmpdir);
    int status = run_program(myargv);
    if (status != RC_OKAY) {
      cleanup();
      exit(RC_SYSTEM_ERROR);
    }
#endif
  }

#ifndef USE_MAKE
  char file_buf1[100];
  if (Ipa_Label_Suffix[0] == '\0') {
    sprintf(file_buf1, "-TENV:ipa_suffix=%d_%d", ipa_time, getpid());
  } else {
    sprintf(file_buf1, "");
  }

  char file_buf2[strlen(Ipa_Exec_Name) + strlen("-TENV::ipa_exec_name=") + 1];
  sprintf(file_buf2, "-TENV::ipa_exec_name=%s", Ipa_Exec_Name);
#endif

  // For each whirl file, tell how to create the corresponding elf file.
  for (size_t i = 0; i < infiles->size(); ++i) {
#ifdef USE_MAKE
    fprintf(makefile, "%s/%s" TARGET_DELIMITER "%s/%s %s/%s %s/%s\n",
            tmpdir_macro, (*outfiles)[i],
            tmpdir_macro, elf_symtab_name,
            tmpdir_macro, whirl_symtab_name,
            tmpdir_macro, (*infiles)[i]);
#if defined(TARG_IA64)
    fprintf(makefile, "\tcd %s; %s %s\n",
            tmpdir_macro, (*commands)[i], extra_args);
#elif defined(TARG_ST)
    if (Ipa_Label_Suffix[0] == '\0') {
      fprintf(makefile, "\tcd %s; %s%s -TENV:ipa_suffix=%d_%d -TENV::ipa_exec_name=%s %s\n",
	      tmpdir_macro, relativepath, (*commands)[i], ipa_time, getpid(),
	      Ipa_Exec_Name, extra_args);
    } else {
      fprintf(makefile, "\tcd %s; %s%s -TENV::ipa_exec_name=%s %s\n",
	      tmpdir_macro, relativepath, (*commands)[i], 
	      Ipa_Exec_Name, extra_args);
    }
#else
    fprintf(makefile, "\tcd -P %s; %s %s\n",
            tmpdir_macro, (*commands)[i], extra_args);
#endif

#else /* USE_MAKE */
    ARGV myargv;
    string2argv(relativepath, myargv);
    string2argv((*commands)[i], myargv);
    myargv.push_back(file_buf1);
    myargv.push_back(file_buf2);
    string2argv(extra_args, myargv);

    int status = run_program(myargv);
    if (status != RC_OKAY) {
      cleanup();
      exit(RC_SYSTEM_ERROR);
    }
#endif /* USE_MAKE */

#ifdef USE_MAKE
    const std::vector<const char*>& com = (*comments)[i];
    for (std::vector<const char*>::const_iterator it = com.begin();
         it != com.end();
         ++it)
      fprintf(makefile, "## %s\n", *it);
    fputs("\n", makefile);
#endif
  }

#ifndef USE_MAKE
	  // [CL] get back to original dir before link step
#ifdef TARG_ST
    if (ld_ipa_opt[LD_IPA_SHOW].flag || ld_ipa_opt[LD_IPA_SHOW_ONLY].flag) {
#else
    if (ld_ipa_opt[LD_IPA_SHOW].flag) {
#endif
      fprintf(stderr, "cd %s\n", current_dir);
    }
  int res = chdir(current_dir);
  if (res) {
    fprintf(stderr, "Couldn't change dir to %s: ", current_dir);
    perror("");
  }
  int status = run_program(link_argv);
  if (status != RC_OKAY) {
    cleanup();
    exit(RC_SYSTEM_ERROR);
  }
  cleanup();
#endif

#ifdef USE_MAKE
  fclose(makefile);
#endif
  if (Tlog_File_Name) {
    fclose (Tlog_File);  
  }

#ifndef USE_MAKE
  exit(0);
#else
  // We don't call make directly.  Instead we call sh, and have it
  // call sh.  This makes cleanup simpler.
  char sh_cmdfile_buf[256];
  sprintf(sh_cmdfile_buf, "cmdfile.%ld", (long) getpid());
  char* sh_cmdfile_name = create_unique_file(sh_cmdfile_buf, 0);

  FILE* sh_cmdfile = fopen(sh_cmdfile_name, "w");
  if (sh_cmdfile == 0)
    ErrMsg (EC_Ipa_Open, sh_cmdfile_name, sys_errlist[errno]);
  chmod(sh_cmdfile_name, 0644);  

    // ensure MAKEFLAGS is not passed to smake(or make).
    // MAKEFLAGS can only cause trouble if passed in,
    // and gmake passes CCTYPE=  information into
    // MAKEFLAGS.  A problem 
    // for smake if CCTYPE=-Ofast is used, as the 't'
    // is interpreted as the MAKEFLAGS 'touch only'
    // option of smake.
    // Using the simplest, most universal env var reset
    // command format, that of plain old sh.
  fprintf(sh_cmdfile,"#! /bin/sh -f \n");
  fprintf(sh_cmdfile,"MAKEFLAGS=\nexport MAKEFLAGS\n");

  // Define a shell function for the cleanup.
  if (!ld_ipa_opt[LD_IPA_KEEP_TEMPS].flag) {
    fprintf(sh_cmdfile, "cleanup() {\n");
    // Remove each temporary file that we know about.
    std::vector<const char*>::iterator i;
    for (i = infiles->begin(); i != infiles->end(); ++i)
      fprintf(sh_cmdfile, "if [ -f %s/%s ] ; then 'rm' -f %s/%s ; fi\n",
              tmpdir, *i, tmpdir, *i);
    for (i = outfiles->begin(); i != outfiles->end(); ++i)
      fprintf(sh_cmdfile, "if [ -f %s/%s ] ; then 'rm' -f %s/%s ; fi\n",
              tmpdir, *i, tmpdir, *i);

    if (strlen(input_symtab_name) != 0) {
      Is_True(strlen(whirl_symtab_name) != 0 && strlen(elf_symtab_name) != 0,
              ("Inconsistent symtab names: input, whirl, elf = %d %d %d\n",
               strlen(input_symtab_name),
               strlen(whirl_symtab_name),
               strlen(elf_symtab_name)));      
    
      fprintf(sh_cmdfile, "if [ -f %s/%s ] ; then 'rm' -f %s/%s ; fi\n",
              tmpdir, input_symtab_name, tmpdir, input_symtab_name);
      fprintf(sh_cmdfile, "if [ -f %s/%s ] ; then 'rm' -f %s/%s ; fi\n",
              tmpdir, whirl_symtab_name, tmpdir, whirl_symtab_name);
      fprintf(sh_cmdfile, "if [ -f %s/%s ] ; then 'rm' -f %s/%s ; fi\n",
              tmpdir, elf_symtab_name, tmpdir, elf_symtab_name);
    }

    if (link_cmdfile_name)
      fprintf(sh_cmdfile, "if [ -f %s ] ; then 'rm' -f %s ; fi\n",
              link_cmdfile_name, link_cmdfile_name);

    fprintf(sh_cmdfile,
            "'rm' %s > /dev/null 2>&1 ; true\n", makefile_name);
    fprintf(sh_cmdfile,
            "'rm' %s > /dev/null 2>&1 ; true\n", sh_cmdfile_name);

    // Move any files that we don't know about to cwd.  We use a 
    // complicated shell command because, if no such files exist, we
    // don't want the user to see any diagnostics.
    fprintf(sh_cmdfile, "'mv' %s/* . > /dev/null 2>&1 ; true\n", tmpdir);

#ifndef TARG_ST
    // [CL] it is not possible for the shell script to remove
    // the directory that constains itself!

    // Remove the directory.
    fprintf(sh_cmdfile, "'rm' -rf %s > /dev/null 2>&1 ; true\n", tmpdir);
#endif
    fprintf(sh_cmdfile, "}\n\n");   // End of cleanup function.

    // Establish a signal handler so that cleanup always gets called.
    // SEGV should be here too; we're leaving it out because 6.2 sh doesn't
    // like it.
    fprintf(sh_cmdfile, "trap 'cleanup; exit 2' ");
#ifndef TARG_IA64
#ifdef linux
    // [CL] Linux does not support EML & POLL
    fprintf(sh_cmdfile, "ABRT SYS ");
#elif defined(__CYGWIN__)
    fprintf(sh_cmdfile, "%d %d %d %d ", SIGABRT, SIGEMT, SIGSYS, SIGPOLL);
#else
    fprintf(sh_cmdfile, "ABRT EMT SYS POLL ");
#endif
#endif
#if defined(__CYGWIN__)
    fprintf(sh_cmdfile, "%d %d %d %d %d %d ", SIGHUP, SIGINT, SIGQUIT, SIGILL, SIGTRAP, SIGFPE);
    fprintf(sh_cmdfile, "%d %d %d %d %d ", SIGKILL, SIGBUS, SIGPIPE, SIGALRM, SIGTERM);
    fprintf(sh_cmdfile, "%d %d %d %d %d %d %d\n\n\n", SIGUSR1, SIGUSR2, SIGIO, SIGVTALRM, SIGPROF, SIGXCPU, SIGXFSZ);
#else
    fprintf(sh_cmdfile, "HUP INT QUIT ILL TRAP FPE ");
    fprintf(sh_cmdfile, "KILL BUS PIPE ALRM TERM ");
    fprintf(sh_cmdfile, "USR1 USR2 IO VTALRM PROF XCPU XFSZ\n\n\n");
#endif
  }

  // Call smake.
  Is_True(command_map != 0
          && command_map->find(MAKE_STRING) != command_map->end()
          && (*command_map)[MAKE_STRING] != 0
          && strlen((*command_map)[MAKE_STRING]) != 0,
          ("Full pathname for smake not set up"));
  const char* smake_name = (*command_map)[MAKE_STRING];
  fprintf(sh_cmdfile, "%s -f %s ", smake_name, makefile_name);

#if 1
#ifdef TARG_ST
    if (ld_ipa_opt[LD_IPA_SHOW].flag || ld_ipa_opt[LD_IPA_SHOW_ONLY].flag) 
#else
    if (ld_ipa_opt[LD_IPA_SHOW].flag) 
#endif
#else
  if (!IPA_Echo_Commands)
#endif
    fprintf(sh_cmdfile, "-s ");

  if (IPA_Max_Jobs_Set) 
#ifndef TARG_ST // [CL] change for GNU make
    fprintf(sh_cmdfile, "-J %u ", IPA_Max_Jobs);
#else
#ifdef _USE_GNU_MAKE_
    fprintf(sh_cmdfile, "-j %u ", IPA_Max_Jobs);
#else
    fprintf(sh_cmdfile, "-J %u ", IPA_Max_Jobs);
#endif
#endif

  fprintf(sh_cmdfile, "\nretval=$?\n");


  // Do cleanup, and return.
  if (!ld_ipa_opt[LD_IPA_KEEP_TEMPS].flag) {
    fprintf(sh_cmdfile, "cleanup; ");
  }
  fprintf(sh_cmdfile, "exit $retval\n");

  fclose(sh_cmdfile);

#ifdef TARG_ST
  // [CL] create a wrapper script so that we can remove
  // tmpdir

  // We don't call make directly.  Instead we call sh, and have it
  // call sh.  This makes cleanup simpler.
  char sh_wrappercmdfile_buf[256];
  if (!ld_ipa_opt[LD_IPA_KEEP_TEMPS].flag) {
    sprintf(sh_wrappercmdfile_buf, "%s.cmdfile.%ld", outfilename, (long) getpid());
  } else {
    sprintf(sh_wrappercmdfile_buf, "%s.cmdfile", outfilename);
    /* If it already exists, create_unique_file_in_curdir() will
       generate a new name, so delete the file if it already
       exists. Ignore errors, as we don't care: if the file does not
       exist, it is not an error. If it couldn't be removed,
       create_unique_file_in_curdir() will generate another name */
    unlink(sh_wrappercmdfile_buf);
  }
  char* sh_wrappercmdfile_name = create_unique_file_in_curdir(sh_wrappercmdfile_buf, 0);

  FILE* sh_wrappercmdfile = fopen(sh_wrappercmdfile_name, "w");
  if (sh_wrappercmdfile == 0)
    ErrMsg (EC_Ipa_Open, sh_wrappercmdfile_name, sys_errlist[errno]);
  chmod(sh_wrappercmdfile_name, 0644);  

  fprintf(sh_wrappercmdfile,"#! /bin/sh -f \n");
  fprintf(sh_wrappercmdfile, "sh %s\n", sh_cmdfile_name);
  fprintf(sh_wrappercmdfile, "status=$?\n");

  if (!ld_ipa_opt[LD_IPA_KEEP_TEMPS].flag) {
    fprintf(sh_wrappercmdfile, "'rm' -rf %s %s > /dev/null 2>&1 ; true\n",
	    tmpdir, sh_wrappercmdfile_name);
  }

  fprintf(sh_wrappercmdfile, "exit $status\n");
  fclose(sh_wrappercmdfile);

  exec_smake(sh_wrappercmdfile_name);
#else
  exec_smake(sh_cmdfile_name);
#endif

#endif /* USE_MAKE */

} // ipacom_doit

// Helper function for get_extra_args.
static void escape_char (char *str)
{
  char *p = str + 1;

  do {
    *str++ = *p++;
  } while (*str != 0);
} /* escape_char */


// Collect any extra arguments that we will tack onto the command line.
// First collect them as a vector of strings, then concatenate them all
// together into a single string.
static const char* get_extra_args(const char* ipaa_filename)
{
  std::vector<const char*> args;
  args.reserve(16);
  
#ifdef TARG_ST200
  if (Target_ABI == ABI_ST200_embedded) {
    switch (ld_ipa_opt[LD_IPA_SHARABLE].flag) {
    case F_MAKE_SHARABLE:
      args.push_back("-fpic");
      break;
    case F_CALL_SHARED:
    case F_CALL_SHARED_RELOC:
      args.push_back("-fpic");
      break;
    case F_NON_SHARED:
      args.push_back("-fembedded");
      break;
    case F_RELOCATABLE:
      if (IPA_Enable_Relocatable_Opt == TRUE)
	args.push_back("-fpic");
      break;
    }
  } else if (Target_ABI == ABI_ST200_PIC) {
    switch (ld_ipa_opt[LD_IPA_SHARABLE].flag) {
    case F_MAKE_SHARABLE:
      args.push_back("-fpic-caller-sets-gp");
      break;
    case F_CALL_SHARED:
    case F_CALL_SHARED_RELOC:
      args.push_back("-fcpic-caller-sets-gp");
      break;
    case F_NON_SHARED:
      args.push_back("-fembedded");
      break;
    case F_RELOCATABLE:
      if (IPA_Enable_Relocatable_Opt == TRUE)
	args.push_back("-fcpic-caller-sets-gp");
      break;
    }
  }
#else
#ifdef TARG_STxP70
#else
  switch (ld_ipa_opt[LD_IPA_SHARABLE].flag) {
  case F_MAKE_SHARABLE:
    args.push_back("-pic2");
    break;
  case F_CALL_SHARED:
  case F_CALL_SHARED_RELOC:
    args.push_back("-pic1");
    break;
  case F_NON_SHARED:
    args.push_back("-non_shared");
    break;
  case F_RELOCATABLE:
    if (IPA_Enable_Relocatable_Opt == TRUE)
      args.push_back("-pic1");
    break;
  }
#endif
#endif
  
#if 1
  // -IPA:keeplight:=ON, which is the default, means that we keep only
  // the .I files, not the .s files.
  if (ld_ipa_opt[LD_IPA_KEEP_TEMPS].flag && !IPA_Enable_Keeplight)
    args.push_back("-keep");
#endif

  if (ld_ipa_opt[LD_IPA_SHOW].flag)
    args.push_back("-show");

#ifdef TODO
  if (IPA_Enable_Cord) {
    args.push_back("-cord");
    args.push_back(cord_output_file_name);
    args.push_back(call_graph_file_name);
    args.push_back(cord_obj_file_name);
  }
#endif

  /* If there's an IPAA intermediate file, let WOPT know: */
  if (ipaa_filename) {
    char* buf = (char*) malloc(strlen(ipaa_filename) + 32);
    if (!buf)
      ErrMsg (EC_No_Mem, "extra_args");

    sprintf(buf, "-WOPT:ipaa:ipaa_file=%s", ipaa_filename );
    args.push_back(buf);
  }

  /* If there are -WB,... options, pull them out and add them to the
     * list.  Strip the '-WB,', and treat non-doubled internal commas
     * as delimiters for new arguments (undoubling the doubled ones):
     */
  if (WB_flags) {
    string p = ipc_copy_of (WB_flags);
    while (*p) {
      args.push_back(p);
#if 1
      while (*p) {
        if (*p == ',') {
          if (p[1] != ',') {
            *p++ = 0;
            break;
          }
          else
            escape_char(p);
        }
        else if (p[0] == '\\' && p[1] != 0)
          escape_char (p);
        p++;
      }
#endif
    }
  }

  /* If there are -Yx,... options, pull them out and add them to the
     * list.  Several may be catenated with space delimiters:
     */
  std::vector<char*> space_ptr; // for restoring spaces overwritten by zero
  if (Y_flags) {
    char* p = Y_flags;
    while (*p) {
      args.push_back(p);
      while (*p) {
        if (*p == ' ') {
          space_ptr.push_back(p);
          *p++ = 0;
          break;
        }
        else if (p[0] == '\\')
          escape_char (p);
        p++;
      }
    }
  }

#ifdef _TARG_MIPS
  /* If there is a -mips[34] option, add it. */
  if (ld_ipa_opt[LD_IPA_ISA].set) {
    switch(ld_ipa_opt[LD_IPA_ISA].flag) {
    case 3:
      args.push_back("-mips3");
      break;
    case 4:
      args.push_back("-mips4");
      break;
    default:
      break;
    }
  }
#endif

  size_t len = 0;
  std::vector<const char*>::const_iterator i;

  for (i = args.begin(); i != args.end(); ++i)
    len += strlen(*i) + 1;

  char* result = (char*) malloc(len + 1);
  if (!result)
    ErrMsg (EC_No_Mem, "extra_args");    

  result[0] = '\0';

  for (i = args.begin(); i != args.end(); ++i) {
    strcat(result, *i);
    strcat(result, " ");
  }

  // now restore spaces in Y_flags that were overwritten by zeros
  for (size_t idx = 0; idx < space_ptr.size(); idx++) {
    Is_True(*space_ptr[idx] == 0, ("space_ptr must point to 0"));
    *space_ptr[idx] = ' ';
  }

  return result;
} /* get_extra_args */

static const char* get_extra_symtab_args(const ARGV& argv)
{
  const char* result = get_extra_args(0);

  for (ARGV::const_iterator i = argv.begin(); i != argv.end(); ++i) {
    const char* const debug_flag = "-DEBUG";
    const char* const G_flag = "-G";
    const char* const TARG_flag = "-TARG";
    const char* const OPT_flag = "-OPT";
#ifdef TARG_ST
    // [CL] add support for target and endianness selection
    const char* const MCORE_flag = "-mcore";
    const char* const BigEndian_flag = "-EB";
    const char* const LittleEndian_flag = "-EL";
#endif
    const int debug_len = 6;
    const int G_len = 2;
    const int TARG_len = 5;
    const int OPT_len = 4;
#ifdef TARG_ST
    const int MCORE_len = 6;
    const int BigEndian_len = 3;
    const int LittleEndian_len = 3;
#endif
    bool flag_found = false;

    // The link line contains -r.  That means we don't have enough information
    // from the link line alone to determine whether the symtab should be
    // compiled shared or nonshared.  We have to look at how one of the other
    // files was compiled.
    if (ld_ipa_opt[LD_IPA_SHARABLE].flag == F_RELOCATABLE &&
                IPA_Enable_Relocatable_Opt != TRUE) {
      const char* const non_shared_flag = "-non_shared";
      if (strcmp(*i, non_shared_flag) == 0)
        flag_found = true;
    }

    if ((strncmp(*i, debug_flag, debug_len) == 0) ||
            (strncmp(*i, G_flag, G_len) == 0) ||
            (strncmp(*i, OPT_flag, OPT_len) == 0) ||
#ifdef TARG_ST
            (strncmp(*i, TARG_flag, TARG_len) == 0) ||
            (strncmp(*i, MCORE_flag, MCORE_len) == 0) ||
            (strncmp(*i, BigEndian_flag, BigEndian_len) == 0) ||
            (strncmp(*i, LittleEndian_flag, LittleEndian_len) == 0)
#else
            (strncmp(*i, TARG_flag, TARG_len) == 0)
#endif
	)

      flag_found = true;

    if (flag_found == true) {
      char* buf = static_cast<char*>(malloc(strlen(result) +
                                            strlen(*i) + 2));
      if (!buf)
        ErrMsg (EC_No_Mem, "extra_symtab_args");
      strcpy(buf, result);
      strcat(buf, " ");
      strcat(buf, *i);
      free(const_cast<char*>(result));
      result = buf;
     }
  }

  return result;
}

#ifdef USE_MAKE
static void exec_smake (char* sh_cmdfile_name)
{
  /* Clear the trace file: */
  Set_Trace_File ( NULL );

#ifdef _OPENMP
  /* The exec process will cause mp slaves, if any, to
       be killed.  They will in turn send a signal to the
       master process, which is the new process.  This process
       in turn has a signal handler that dies when it finds out
       that its slaves die.  This way, if we ever compile -mp,
       we kill the child processes before doing the exec */
  mp_destroy_();
#endif

  // Call the shell.
  char* sh_name = "/bin/sh";

  const int argc = 2;
  char* argv[argc+1];
  argv[0] = sh_name;
  argv[1] = sh_cmdfile_name;
  argv[2] = 0;
  
  execve (sh_name, argv, environ_vars);

  // if the first try fails, use the user's search path
  execvp ("sh", argv);

  Fail_FmtAssertion ("ipa: exec sh failed");
}
#endif
