/*

  Copyright (C) 2000 Silicon Graphics, Inc.  All Rights Reserved.

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
/* This file has been modifed by STMicroelectronics */

// Note: currently, enumerations are not supported
// in dynamic extensions. We raise an error message if so.



// isa_enums_gen.cxx
/////////////////////////////////////
//
//  Generate a list of enum classes and their values.
//
/////////////////////////////////////
//

#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <assert.h>
// [HK]
#if __GNUC__ >=3 || defined(_MSC_VER)
#include <vector>
using std::vector;
#else
#include <vector.h>
#endif // __GNUC__ >=3 || defined(_MSC_VER)
#include "gen_util.h"
#include "isa_enums_gen.h"

typedef struct {
	const char *ecv_ecname;
	const char *ecv_name;
	int ecv_int;
} ECV_struct;

typedef struct {
	const char *ec_name;
	int first_ecv;
	int last_ecv;
} EC_struct;

static vector<ECV_struct> all_ecv;
static vector<EC_struct> all_ec;

static const char * const interface[] = {
  "/* ====================================================================",
  " * ====================================================================",
  " *",
  " * Description:",
  " *",
  " *   A list of all the enum classes used in an ISA.",
  " *   It exports the following:",
  " *",
  " *   typedef (enum) ISA_ENUM_CLASS",
  " *       An enumeration of the enum classes.",
  " *",
  " *   typedef (enum) ISA_ENUM_CLASS_VALUE",
  " *       An enumeration of the enum class values.",
  " *",
  " *   typedef (struct) ISA_ENUM_CLASS_INFO",
  " *       Contains info about first and last ECV in the EC.",
  " *       The contents are private.",
  " *",
  " *   typedef (struct) ISA_ENUM_CLASS_VALUE_INFO",
  " *       Contains info about name and int-value of the ECV.",
  " *       The contents are private.",
  " *",
  " *   const char * ISA_EC_Name (ISA_ENUM_CLASS)",
  " *       Returns name of EC.",
  " *",
  " *   ISA_ENUM_CLASS_VALUE ISA_EC_First_Value (ISA_ENUM_CLASS)",
  " *       Returns the first ECV for the specified EC.",
  " *",
  " *   ISA_ENUM_CLASS_VALUE ISA_EC_Last_Value (ISA_ENUM_CLASS)",
  " *       Returns the last ECV for the specified EC.",
  " *       Note that it assumes all ECV for an EC are in the",
  " *       first/last range given by the above two functions.",
  " *",
  " *   const char * ISA_ECV_Name (ISA_ENUM_CLASS_VALUE)",
  " *       Returns name of ECV.",
  " *",
  " *   INT ISA_ECV_Intval (ISA_ENUM_CLASS_VALUE)",
  " *       Returns int-value of ECV.",
  " *",
  " * ====================================================================",
  " * ====================================================================",
  " */",
  NULL
};

static bool gen_static_code = true;          // Whether we generate code for an
static bool gen_dyn_code    = false;         // extension or for the core.

static char *extname        = NULL;          // Extension name (NULL if no ext).



/////////////////////////////////////
void ISA_Enums_Begin (void)
/////////////////////////////////////
//  See interface description.
/////////////////////////////////////
{
  gen_static_code = Is_Static_Code();
  gen_dyn_code    =!gen_static_code;

   if(gen_dyn_code)
     extname = Get_Extension_Name();

  // start with undefined value
  // when code is generated for
  // the core.
  if(gen_static_code) {
    ECV_struct current_ecv = {"","UNDEFINED",UNDEFINED};
    all_ecv.push_back (current_ecv);
    EC_struct current_ec = {"UNDEFINED",0,0};
    all_ec.push_back (current_ec);
  }

  return;
}

/////////////////////////////////////
void ISA_Create_Enum_Class ( const char* name, ...)
/////////////////////////////////////
//  See interface description.
/////////////////////////////////////
{
  const char *ecv_name;
  va_list ap;
  EC_struct current_ec;
  ECV_struct current_ecv;
  current_ec.ec_name = name;
  current_ec.first_ecv = all_ecv.size();
  va_start(ap, name);
  do {	// loop through ecvs
    ecv_name = va_arg(ap, char*);
    current_ecv.ecv_ecname = name;
    current_ecv.ecv_name = ecv_name ? ecv_name : "";
    current_ecv.ecv_int = va_arg(ap, int);
    if (current_ecv.ecv_int == UNDEFINED)
	break;	// no default value
    all_ecv.push_back (current_ecv);
  } while (ecv_name != NULL);
  va_end(ap);
  current_ec.last_ecv = all_ecv.size() - 1;
  all_ec.push_back (current_ec);
}


static char*
Print_ECV_EName (const char *name)
{
  // will print direct to file, so can use temp buffers
  static char buf[80];
  char *p = (char*) name;
  int i = 0;
  if (name == NULL)
    return "_none";
  else if (name[0] != '\0' && name[0] != '.' && name[0] != '_') {
    // insert leading _
    buf[0] = '_';
    ++i;
  }
  for ( ; *p != '\0'; ++p) {
    switch (*p) {
    case '.':
      buf[i++] = '_';
      break;
    case '@':
      // remove from name
      break;
    default:
      buf[i++] = *p;
      break;
    }
  }
  buf[i] = '\0';
  return buf;
}

/////////////////////////////////////
void ISA_Enums_End(void)
/////////////////////////////////////
//  See interface description.
/////////////////////////////////////
{
  char *hfilename = NULL ;
  char *cfilename = NULL ;
  char *efilename = NULL ;
  FILE *hfile     = NULL ;
  FILE *cfile     = NULL ;
  FILE *efile     = NULL ;

  const char* const bname = FNAME_TARG_ISA_ENUMS ;      /* base name */

  const char* tabname;
  const char* ptrname; 

  vector<EC_struct>::iterator iec;
  vector<ECV_struct>::iterator iecv;
  ECV_struct tecv;

  hfilename = Gen_Build_Filename(bname,extname,gen_util_file_type_hfile);
  hfile     = Gen_Open_File_Handle(hfilename, "w");

  cfilename = Gen_Build_Filename(bname,extname,gen_util_file_type_cfile);
  cfile     = Gen_Open_File_Handle(cfilename, "w");

  if(gen_static_code)
   { efilename = Gen_Build_Filename(bname,extname,gen_util_file_type_efile);
     efile     = Gen_Open_File_Handle(efilename, "w");
   }

  Emit_Header (hfile, bname, interface,extname);
  Emit_C_Header(cfile);

  if(gen_dyn_code)
   { char *dyn_hfilename = Gen_Build_Filename(FNAME_ISA_ENUMS,NULL,
                                              gen_util_file_type_dyn_hfile);
     fprintf(cfile,"#include <stdio.h>\n");
     fprintf(cfile,"#include \"%s\"\n",dyn_hfilename);
     Gen_Free_Filename(dyn_hfilename);
   }
  else
   { fprintf(cfile,"#include \"%s\"\n\n",hfilename);
   }

  // Currently enumerations are not supported for dynamic extensions.
  if (gen_dyn_code)
   {if(all_ecv.begin() != all_ecv.end() ||
       all_ec.begin()  != all_ec.end())
      { fprintf(stderr,"### Error: enumerations are not supported yet\n");
        exit(EXIT_FAILURE);
      }

    const char* proto1 = 
   "const ISA_ENUM_CLASS_INFO* dyn_get_ISA_ENUM_CLASS_INFO_tab(void)";
    const char* proto2 =
   "const mUINT32 dyn_get_ISA_ENUM_CLASS_INFO_tab_sz(void)";
    const char* proto3 =
   "const ISA_ENUM_CLASS_VALUE_INFO* dyn_get_ISA_ENUM_CLASS_VALUE_INFO_tab(void)";
    const char* proto4 =
   "const mUINT32 dyn_get_ISA_ENUM_CLASS_VALUE_INFO_tab_sz(void)";


    /* Empty routines */
    fprintf(cfile,
            "%s {\n"
            " return (const ISA_ENUM_CLASS_INFO*)NULL;\n"
            "};\n"
            "\n"
            "%s {\n"
            " return (const mUINT32)0U;\n"
            "};\n"
            "\n"
            "%s {\n"
            " return (const ISA_ENUM_CLASS_VALUE_INFO*)NULL;\n"
            "};\n"
            "\n"
            "%s {\n"
            " return (const mUINT32)0U;\n"
            "};\n"
            "\n",
            proto1,
            proto2,
            proto3,
            proto4);
    fprintf(hfile,
            "%s;\n"
            "%s;\n"
            "%s;\n"
            "%s;\n"
            "\n",
            proto1,
            proto2,
            proto3,
            proto4);

     goto end;
   }

  fprintf(hfile, "\nenum {\n");
  for ( iec = all_ec.begin(); iec != all_ec.end(); ++iec) {
  	fprintf(hfile, "\tISA_EC%s,\n", Print_ECV_EName(iec->ec_name));
  }
  fprintf(hfile, "\tISA_EC_STATIC_MAX\n");
  fprintf(hfile, "};\n");
  fprintf(hfile, "typedef mUINT32 ISA_ENUM_CLASS;\n\n");
  fprintf(hfile, "BE_EXPORTED extern mUINT32 ISA_EC_MAX;\n\n");
  fprintf(cfile, "BE_EXPORTED mUINT32 ISA_EC_MAX = ISA_EC_STATIC_MAX;\n\n");

  fprintf(hfile, "\nenum {\n");
  for ( iecv = all_ecv.begin(); iecv != all_ecv.end(); ++iecv) {
	// have to use multiple calls since Print_ECV_EName uses a static bufr
  	fprintf(hfile, "\tISA_ECV%s", Print_ECV_EName (iecv->ecv_ecname));
  	fprintf(hfile, "%s,\n", Print_ECV_EName (iecv->ecv_name));
  }
  fprintf(hfile, "\tISA_ECV_STATIC_MAX\n");
  fprintf(hfile, "};\n");
  fprintf(hfile, "typedef mUINT32 ISA_ENUM_CLASS_VALUE;\n\n");
  fprintf(hfile, "BE_EXPORTED extern mUINT32 ISA_ECV_MAX;\n\n");
  fprintf(cfile, "BE_EXPORTED mUINT32 ISA_ECV_MAX = ISA_ECV_STATIC_MAX;\n\n");

  /*==================================================================*/

  tabname = "ISA_ENUM_CLASS_info_static_tab";
  ptrname = "ISA_ENUM_CLASS_info";
    
  fprintf(hfile, "\ntypedef struct {\n"
		"  char *name;\n"
		"  ISA_ENUM_CLASS_VALUE first;\n"
		"  ISA_ENUM_CLASS_VALUE last;\n"
		"} ISA_ENUM_CLASS_INFO;\n");
  fprintf(cfile, "static const ISA_ENUM_CLASS_INFO %s[] = {\n",tabname);
  for ( iec = all_ec.begin(); iec != all_ec.end(); ++iec) {
  	fprintf(cfile, "\t{ \"ISA_EC%s\",", Print_ECV_EName(iec->ec_name));
	tecv = all_ecv[iec->first_ecv];
	// have to use multiple calls since Print_ECV_EName uses a static bufr
  	fprintf(cfile, "\tISA_ECV%s", Print_ECV_EName(tecv.ecv_ecname));
  	fprintf(cfile, "%s,", Print_ECV_EName(tecv.ecv_name));
	tecv = all_ecv[iec->last_ecv];
	// have to use multiple calls since Print_ECV_EName uses a static bufr
  	fprintf(cfile, "\tISA_ECV%s", Print_ECV_EName(tecv.ecv_ecname));
  	fprintf(cfile, "%s },\n", Print_ECV_EName(tecv.ecv_name));
  }
  fprintf(cfile, "};\n");

  fprintf(hfile, 
          "BE_EXPORTED extern const ISA_ENUM_CLASS_INFO *%s;\n",
          ptrname);
  fprintf(efile, 
          "%s\n",
          ptrname);
  fprintf(cfile,
          "BE_EXPORTED const ISA_ENUM_CLASS_INFO *%s = %s;\n\n",
          ptrname,tabname);

  /*==================================================================*/
  
  tabname = "ISA_ENUM_CLASS_VALUE_info_static_tab";
  ptrname = "ISA_ENUM_CLASS_VALUE_info";

  fprintf(hfile, "\ntypedef struct {\n"
		"  char *name;\n"
		"  INT intval;\n"
		"} ISA_ENUM_CLASS_VALUE_INFO;\n");
  fprintf(cfile, "static const ISA_ENUM_CLASS_VALUE_INFO %s[] = {\n",tabname);
  for ( iecv = all_ecv.begin(); iecv != all_ecv.end(); ++iecv) {
  	fprintf(cfile, "\t{ \"%s\",\t%d },\n", iecv->ecv_name, iecv->ecv_int);
  }
  fprintf(cfile, "};\n");

  fprintf(hfile, 
          "BE_EXPORTED extern const ISA_ENUM_CLASS_VALUE_INFO *%s;\n",
          ptrname);
  fprintf(efile, 
          "%s\n",
          ptrname);
  fprintf(cfile,
          "BE_EXPORTED const ISA_ENUM_CLASS_VALUE_INFO *%s = %s;\n",
           ptrname,tabname);

  /*==================================================================*/

  fprintf(hfile, "inline const char * ISA_EC_Name (ISA_ENUM_CLASS ec)\n"
		 "{\n"
		 "  return ISA_ENUM_CLASS_info[ec].name;\n"
		 "}\n\n");

  fprintf(hfile, "inline ISA_ENUM_CLASS_VALUE ISA_EC_First_Value (ISA_ENUM_CLASS ec)\n"
		 "{\n"
		 "  return ISA_ENUM_CLASS_info[ec].first;\n"
		 "}\n\n");

  fprintf(hfile, "inline ISA_ENUM_CLASS_VALUE ISA_EC_Last_Value (ISA_ENUM_CLASS ec)\n"
		 "{\n"
		 "  return ISA_ENUM_CLASS_info[ec].last;\n"
		 "}\n\n");

  fprintf(hfile, "inline const char * ISA_ECV_Name (ISA_ENUM_CLASS_VALUE ecv)\n"
		 "{\n"
		 "  return ISA_ENUM_CLASS_VALUE_info[ecv].name;\n"
		 "}\n\n");

  fprintf(hfile, "inline INT ISA_ECV_Intval (ISA_ENUM_CLASS_VALUE ecv)\n"
		 "{\n"
		 "  return ISA_ENUM_CLASS_VALUE_info[ecv].intval;\n"
		 "}\n\n");


  end:

  Emit_Footer (hfile);
  Emit_C_Footer(cfile);


  // Closing file handlers.
  if(hfile) Gen_Close_File_Handle(hfile,hfilename);
  if(cfile) Gen_Close_File_Handle(cfile,cfilename);
  if(efile) Gen_Close_File_Handle(efile,efilename);

  // Memory deallocation.
  if(cfilename) Gen_Free_Filename(cfilename);
  if(hfilename) Gen_Free_Filename(hfilename);
  if(efilename) Gen_Free_Filename(efilename);

  return;
}
