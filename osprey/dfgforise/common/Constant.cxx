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
////    Definition of Constant class and functions.                              ////
////                                                                             ////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////


/**
 * \file Constant.cxx
 */


#include "Constant.h"

#include "OperandResult.h"

#include <string>
#include <sstream>



namespace DfgForIse {



  ///////////////////////////////////////////////////////////////////////////////////
  ////    Constructor.                                                           ////
  ///////////////////////////////////////////////////////////////////////////////////

  Constant::Constant(const long long value, const size_t bitSize) {
    mBitSize = bitSize;
    mValue = value;
  };



  ///////////////////////////////////////////////////////////////////////////////////
  ////    Set and get functions.                                                 ////
  ///////////////////////////////////////////////////////////////////////////////////

  bool Constant::isVariable() const {
    return false;
  };


  long long Constant::getValue() const {
    return mValue;
  };


  std::string Constant::getName() const {
    std::ostringstream result;
    result << mValue;
    return result.str();
  };



  ///////////////////////////////////////////////////////////////////////////////////
  ////    Dumping functions.                                                     ////
  ///////////////////////////////////////////////////////////////////////////////////
  

  std::string Constant::getHtmlDescription() const {
    std::ostringstream result;
    result << "<u><b>Constant :</b></u>"
	   << "<br>";
    result << "Value : " << getValue()
	   << "<br>";
    result << "Size : " << getBitSize() << " bits";
    return result.str();
  };


  std::string Constant::getXmlDescription() const {
    std::ostringstream result;
    result << getXmlHeader();
    result << "            <ConstantValue>" << getValue()
	   << "</ConstantValue>" << std::endl;
    result << getXmlFooter();
    return result.str();
  };



  ///////////////////////////////////////////////////////////////////////////////////
  ////    Operators.                                                             ////
  ///////////////////////////////////////////////////////////////////////////////////

  bool operator==(const Constant& rConstant1, const Constant& rConstant2) {
    return rConstant1.mValue == rConstant2.mValue;
  };



  ///////////////////////////////////////////////////////////////////////////////////
  ////    Destructor.                                                            ////
  ///////////////////////////////////////////////////////////////////////////////////

  Constant::~Constant() {
  };



};
