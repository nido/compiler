/*
 
  Copyright (C) 2008 ST Microelectronics, Inc.  All Rights Reserved.
 
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
 
  Contact information:  ST Microelectronics, Inc.,
  , or:
                                                                                
  http://www.st.com
                                                                                
  For further information regarding this notice, see:
                                                                                
  http:
*/
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////                                                                             ////
////    Generation of DfgForIse_Opcode.h.                                        ////
////                                                                             ////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <map>

#include "topcode.h"
#include "targ_isa_properties.h"
#include "targ_isa_operands.h"
#include "ti_si.h"
#include "gen_util.h"

#include "dfgforise_opcode_gen.h"


/* Keeps the topcodes which are forbidden in selection. */
std::map<TOP, const char*> gMapTopAssociatedToClass;



void AssociateTopToClass(const TOP topcode, const char* const opclass) {
  gMapTopAssociatedToClass[topcode] = opclass;
};



void DfgForIse_OpCode_Create() {

  TOP topcode;


  // Name of DfgForIse OpCode file.
  const char* const name_dfgforise_opcode_h = Gen_Build_Filename (FNAME_DFGFORISE_OPCODE,
								  NULL,
								  gen_util_file_type_hfile);
  

  // DfgForIse OpCode file.
  FILE* hfile = Gen_Open_File_Handle(name_dfgforise_opcode_h ,"w");


  // Printing header.
  fprintf(hfile, "/////////////////////////////////////////////////////////////////////////////////////\n");
  fprintf(hfile, "/////////////////////////////////////////////////////////////////////////////////////\n");
  fprintf(hfile, "////                                                                             ////\n");
  fprintf(hfile, "////    Declaration of DfgForIse OpCode types.                                   ////\n");
  fprintf(hfile, "////                                                                             ////\n");
  fprintf(hfile, "/////////////////////////////////////////////////////////////////////////////////////\n");
  fprintf(hfile, "/////////////////////////////////////////////////////////////////////////////////////\n");
  fprintf(hfile, "\n");
  fprintf(hfile, "#ifndef DfgForIse_OpCode_h\n");
  fprintf(hfile, "#define DfgForIse_OpCode_h\n");
  fprintf(hfile, "\n");
  fprintf(hfile, "\n");
  fprintf(hfile, "\n");
  fprintf(hfile, "#include \"OpClass.h\"\n");
  fprintf(hfile, "\n");
  fprintf(hfile, "\n");
  fprintf(hfile, "\n");
  fprintf(hfile, "namespace DfgForIse {\n");


  // Printing OpCode enumeration.
  fprintf(hfile, "\n");
  fprintf(hfile, "\n");
  fprintf(hfile, "\n");
  fprintf(hfile, "  /** Enumeration of the opcodes used in the DFG's representation.\n");
  fprintf(hfile, "   */\n");
  fprintf(hfile, "  enum OpCode {\n");
  for (topcode = 0;
       topcode < TOP_count;
       topcode++) {
    fprintf(hfile, "    OPCODE_%s,\n",TOP_Name(topcode));
  }
  fprintf(hfile, "    OPCODE_NUMBER\n");
  fprintf(hfile, "  };\n");


  // Printing OpCodeName Table.
  fprintf(hfile, "\n");
  fprintf(hfile, "\n");
  fprintf(hfile, "\n");
  fprintf(hfile, "  /** Indicates the name to use for each opcode.\n");
  fprintf(hfile, "   */\n");
  fprintf(hfile, "  const char* const OpCodeName[] = {\n");
  for (topcode = STATIC_OFFSET;
       topcode < TOP_count;
       topcode++) {
    fprintf(hfile, "    \"%s\",\t// OPCODE_%s\n",TOP_Name(topcode),TOP_Name(topcode));
  }
  fprintf(hfile, "  };\n");


  // Printing OpCodeSoftwareLatency Table.
  fprintf(hfile, "\n");
  fprintf(hfile, "\n");
  fprintf(hfile, "\n");
  fprintf(hfile, "  /** Indicates the software latency of each opcode.\n");
  fprintf(hfile, "   */\n");
  fprintf(hfile, "  const unsigned int OpCodeSoftwareLatencyCycles[] = {\n");
  for (topcode = 0;
       topcode < TOP_count;
       topcode++) {
    size_t operand_access_time = 0;
    size_t result_available_time = 0;
    if ( SI_top_si[(INT)topcode] && ( gMapTopAssociatedToClass.find(topcode) != gMapTopAssociatedToClass.end() ) ) {
      for (int result = 0;
	   result < ISA_OPERAND_INFO_Results(ISA_OPERAND_Info(topcode));
	   result++) {
	result_available_time = std::max(result_available_time,
				    (size_t)TSI_Result_Available_Time(topcode, result));
      }
      operand_access_time = result_available_time;
      for (int opnd = 0;
	   opnd < ISA_OPERAND_INFO_Operands(ISA_OPERAND_Info(topcode));
	   opnd++) {
	operand_access_time = std::min(operand_access_time,
				       (size_t)TSI_Operand_Access_Time(topcode, opnd));
      }
    }
    fprintf(hfile, "    %d,\t// OPCODE_%s\n", result_available_time - operand_access_time, TOP_Name(topcode));
  }
  fprintf(hfile, "  };\n");


  // Printing OpCodeClass Table.
  fprintf(hfile, "\n");
  fprintf(hfile, "\n");
  fprintf(hfile, "\n");
  fprintf(hfile, "  /** Associates an opclass to each opcode.\n");
  fprintf(hfile, "   */\n");
  fprintf(hfile, "  const OpClass OpCodeClass[] = {\n");
  for (topcode = STATIC_OFFSET;
       topcode < TOP_count;
       topcode++) {
    std::map<TOP, const char*>::const_iterator it_top_class
      = gMapTopAssociatedToClass.find(topcode);
    if (it_top_class == gMapTopAssociatedToClass.end()) {
      fprintf(hfile, "    OPCLASS_FORBIDDEN,\t// OPCODE_%s\n",TOP_Name(topcode));
    }
    else {
      fprintf(hfile, "    OPCLASS_%s,\t// OPCODE_%s\n",it_top_class->second,TOP_Name(topcode));
    }
  }
  fprintf(hfile, "  };\n");


  // Printing footer.
  fprintf(hfile, "\n");
  fprintf(hfile, "\n");
  fprintf(hfile, "\n");
  fprintf(hfile, "};\n");
  fprintf(hfile, "\n");
  fprintf(hfile, "#endif // DfgForIse_OpCode_h\n");


  Gen_Close_File_Handle(hfile, name_dfgforise_opcode_h );


  // Memory deallocation
  Gen_Free_Filename(const_cast<char*> (name_dfgforise_opcode_h));


};
