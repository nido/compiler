/*
  Copyright (C) 2002 ST Microelectronics, Inc.  All Rights Reserved.

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

  Contact information:

  For further information regarding this notice, see:
 */ 
/** 
 * @file   targ_cgdwarf_utils.cxx
 * @author Quentin COLOMBET
 * @date   Wed May 17 15:11:06 2006
 * @brief  This file contains the implementation of the CUnwindElement class
 *         and all its inherited ones.
 * 
 * 
 */

// System include
#include <stdexcept> // For exception creation

// User include
#include "targ_cgdwarf_utils.h" // For classes declaration
#include "errors.h"             // For DevAssert
#include "cxx_memory.h"         // For CXX_NEW
#include "dwarf.h"              // For DW_CFA_xxx definition
#include "cgdwarf.h"            // For CGD_LABIDX


/**
 * This variable represents the size in bit of offset that can be encoding in
 * the DW_CFA_offset
 */
static const INT CFA_OFFSET_ENCODING_SIZE = 6;

/**
 * This varaible represents the default elf section
 */
const ElfSection g_defaultSection = 0;

//-----------------------------------------------------------------------------
// CUnwindElement class definition
//-----------------------------------------------------------------------------
CUnwindElement::CUnwindElement(MEM_POOL* memPool, UINT offsetFromBegin,
                               const ElfSection& section)
{
    MemPool(memPool);
    Offset(offsetFromBegin);
    Section(section);
}

CUnwindElement::CUnwindElement(const CUnwindElement& a_unwindElt)
{
    CopyMembers(a_unwindElt);
}

CUnwindElement&
CUnwindElement::operator=(const CUnwindElement& a_unwindElt)
{
    if(this != &a_unwindElt)
        {
            CopyMembers(a_unwindElt);
        }
    return *this;
}

CUnwindElement::~CUnwindElement()
{
}

void
CUnwindElement::Reloc(const CRelocationInfo& relocInfo)
{
    Offset(Offset() + relocInfo.OpOffset());
    Section(relocInfo.Section());
}

UINT
CUnwindElement::Offset() const
{
    return m_offset;
}

void
CUnwindElement::Offset(UINT offsetFromBegin)
{
    m_offset = offsetFromBegin;
}

const ElfSection&
CUnwindElement::Section() const
{
    return m_section;
}

void
CUnwindElement::Section(const ElfSection& section)
{
    m_section = section;
}

MEM_POOL*
CUnwindElement::MemPool() const
{
    return m_memPool;
}

void
CUnwindElement::MemPool(MEM_POOL* memPool)
{
    m_memPool = memPool;
}

void
CUnwindElement::CopyMembers(const CUnwindElement& a_unwindElt)
{
    Offset(a_unwindElt.Offset());
    Section(a_unwindElt.Section());
    MemPool(a_unwindElt.MemPool());
}

//-----------------------------------------------------------------------------
// CUnwindSaveReg class definition
//-----------------------------------------------------------------------------
CUnwindSaveReg::CUnwindSaveReg(MEM_POOL* memPool, const DebugRegId& regId,
                               INT cfaOffset, UINT offsetFromBegin,
                               const ElfSection& section)
    : CUnwindElement(memPool, offsetFromBegin, section)
{
    RegId(regId);
    CfaOffset(cfaOffset);
}

CUnwindSaveReg::CUnwindSaveReg(const CUnwindSaveReg& a_unwind)
    : CUnwindElement(a_unwind)
{
    CopyMembers(a_unwind);
}

CUnwindSaveReg&
CUnwindSaveReg::operator=(const CUnwindSaveReg& a_unwind)
{
    if(this != &a_unwind)
        {
            static_cast<void>(CUnwindElement::operator=(a_unwind));
            CopyMembers(a_unwind);
        }
    return *this;
}

CUnwindSaveReg::~CUnwindSaveReg()
{
}

Fde
CUnwindSaveReg::AddFdeInst(Fde fde, ErrorDesc* errorDesc) const
{
    // Generate the right dwarf instruction for the given register identifier
//    Dwarf_Small opId = (RegId() > ((1 << CFA_OFFSET_ENCODING_SIZE) - 1))?
//        DW_CFA_offset_extended: DW_CFA_offset;
    // Lib dwarf already manages the encoding problem
    Dwarf_Small opId = DW_CFA_offset;
    return dwarf_add_fde_inst(fde, opId, (Dwarf_Unsigned)RegId(),
                              (Dwarf_Unsigned)CfaOffset(), errorDesc);
}

CUnwindElement*
CUnwindSaveReg::Clone(MEM_POOL* a_mem_pool) const
{
    return CXX_NEW(CUnwindSaveReg(*this), a_mem_pool);
}

void
CUnwindSaveReg::Reloc(const CRelocationInfo& relocInfo)
{
    CUnwindElement::Reloc(relocInfo);
    CfaOffset(CfaOffset() + relocInfo.CfaOffset());
}

BOOL
CUnwindSaveReg::IsSame(const CUnwindElement& ue) const
{
    const CUnwindSaveReg* tmp = dynamic_cast<const CUnwindSaveReg*>(&ue);
    return tmp  && tmp->RegId() == RegId() && tmp->CfaOffset() == CfaOffset();
}

BOOL
CUnwindSaveReg::IsOpposite(const CUnwindElement& ue) const
{
    const CUnwindRestoreReg* tmp = dynamic_cast<const CUnwindRestoreReg*>(&ue);
    return tmp  && tmp->RegId() == RegId();
}


const DebugRegId&
CUnwindSaveReg::RegId() const
{
    return m_regId;
}


INT
CUnwindSaveReg::CfaOffset() const
{
    return m_cfaOffset;
}

void
CUnwindSaveReg::RegId(const DebugRegId& regId)
{
    m_regId = regId;
}

void
CUnwindSaveReg::CfaOffset(INT cfaOffset)
{
    m_cfaOffset = cfaOffset;
}

void
CUnwindSaveReg::CopyMembers(const CUnwindElement& a_unwindElt)
{
    const CUnwindSaveReg* unwindSaveReg =
        dynamic_cast<const CUnwindSaveReg*>(&a_unwindElt);
    if(unwindSaveReg)
        {
            CUnwindElement::CopyMembers(a_unwindElt);
            RegId(unwindSaveReg->RegId());
            CfaOffset(unwindSaveReg->CfaOffset());
        }
    else
        {
            DevAssert(FALSE, ("We should not get here %s %d", __FILE__,
                              __LINE__));
        }
}

//-----------------------------------------------------------------------------
// CUnwindSaveRegNoSp class definition
//-----------------------------------------------------------------------------
CUnwindSaveRegNoSp::CUnwindSaveRegNoSp(MEM_POOL* memPool,
                                       const DebugRegId& regId,
                                       const DebugRegId& saveRegId,
                                       UINT offsetFromBegin,
                                       const ElfSection& section)
    :CUnwindElement(memPool, offsetFromBegin, section)
{
    RegId(regId);
    SaveRegId(saveRegId);
}

CUnwindSaveRegNoSp::CUnwindSaveRegNoSp(const CUnwindSaveRegNoSp& a_unwind)
    : CUnwindElement(a_unwind)
{
    CopyMembers(a_unwind);
}

CUnwindSaveRegNoSp&
CUnwindSaveRegNoSp::operator=(const CUnwindSaveRegNoSp& a_unwind)
{
    if(this != &a_unwind)
        {
            static_cast<void>(CUnwindElement::operator=(a_unwind));
            CopyMembers(a_unwind);
        }
    return *this;
}


CUnwindSaveRegNoSp::~CUnwindSaveRegNoSp()
{
}

Fde
CUnwindSaveRegNoSp::AddFdeInst(Fde fde, ErrorDesc* errorDesc) const
{
    return dwarf_add_fde_inst(fde, DW_CFA_register, (Dwarf_Unsigned)RegId(),
                              (Dwarf_Unsigned)SaveRegId(), errorDesc);
}

CUnwindElement*
CUnwindSaveRegNoSp::Clone(MEM_POOL* a_mem_pool) const
{
    return CXX_NEW(CUnwindSaveRegNoSp(*this), a_mem_pool);
}

BOOL
CUnwindSaveRegNoSp::IsSame(const CUnwindElement& ue) const
{
    const CUnwindSaveRegNoSp* tmp =
        dynamic_cast<const CUnwindSaveRegNoSp*>(&ue);
    return tmp  && tmp->RegId() == RegId() && tmp->SaveRegId() == SaveRegId();
}

BOOL
CUnwindSaveRegNoSp::IsOpposite(const CUnwindElement& ue) const
{
    const CUnwindRestoreRegNoSp* tmp =
        dynamic_cast<const CUnwindRestoreRegNoSp*>(&ue);
    return tmp  && tmp->RegId() == RegId() && tmp->SaveRegId() == SaveRegId();
}

const DebugRegId&
CUnwindSaveRegNoSp::SaveRegId() const
{
    return m_saveRegId;
}

const DebugRegId&
CUnwindSaveRegNoSp::RegId() const
{
    return m_regId;
}


void
CUnwindSaveRegNoSp::SaveRegId(const DebugRegId& saveRegId)
{
    m_saveRegId = saveRegId;
}

void
CUnwindSaveRegNoSp::RegId(const DebugRegId& regId)
{
    m_regId = regId;
}

void
CUnwindSaveRegNoSp::CopyMembers(const CUnwindElement& a_unwindElt)
{
    const CUnwindSaveRegNoSp* unwindSaveRegNoSp =
        dynamic_cast<const CUnwindSaveRegNoSp*>(&a_unwindElt);
    if(unwindSaveRegNoSp)
        {
            CUnwindElement::CopyMembers(a_unwindElt);
            RegId(unwindSaveRegNoSp->RegId());
            SaveRegId(unwindSaveRegNoSp->SaveRegId());
        }
    else
        {
            DevAssert(FALSE, ("We should not get here %s %d", __FILE__,
                              __LINE__));
        }
}

//-----------------------------------------------------------------------------
// CUnwindRestoreReg class definition
//-----------------------------------------------------------------------------
CUnwindRestoreReg::CUnwindRestoreReg(MEM_POOL* memPool, const DebugRegId& regId,
                                     UINT offsetFromBegin,
                                     const ElfSection& section)
    :CUnwindElement(memPool, offsetFromBegin, section)
{
    RegId(regId);
}

CUnwindRestoreReg::CUnwindRestoreReg(const CUnwindRestoreReg& a_unwind)
    : CUnwindElement(a_unwind)
{
    CopyMembers(a_unwind);
}

CUnwindRestoreReg&
CUnwindRestoreReg::operator=(const CUnwindRestoreReg& a_unwind)
{
    if(this != &a_unwind)
        {
            static_cast<void>(CUnwindElement::operator=(a_unwind));
            CopyMembers(a_unwind);
        }
    return *this;
}

CUnwindRestoreReg::~CUnwindRestoreReg()
{
}

Fde
CUnwindRestoreReg::AddFdeInst(Fde fde, ErrorDesc* errorDesc) const
{
    return dwarf_add_fde_inst(fde, DW_CFA_restore, (Dwarf_Unsigned)RegId(),
                              (Dwarf_Unsigned)0, errorDesc);
}

CUnwindElement*
CUnwindRestoreReg::Clone(MEM_POOL* a_mem_pool) const
{
    return CXX_NEW(CUnwindRestoreReg(*this), a_mem_pool);
}

BOOL
CUnwindRestoreReg::IsSame(const CUnwindElement& ue) const
{
    const CUnwindRestoreReg* tmp = dynamic_cast<const CUnwindRestoreReg*>(&ue);
    return tmp  && tmp->RegId() == RegId();
}

BOOL
CUnwindRestoreReg::IsOpposite(const CUnwindElement& ue) const
{
    const CUnwindSaveReg* tmp = dynamic_cast<const CUnwindSaveReg*>(&ue);
    return tmp  && tmp->RegId() == RegId();
}

const DebugRegId&
CUnwindRestoreReg::RegId() const
{
    return m_regId;
}

void
CUnwindRestoreReg::RegId(const DebugRegId& regId)
{
    m_regId = regId;
}

void
CUnwindRestoreReg::CopyMembers(const CUnwindElement& a_unwindElt)
{
    const CUnwindRestoreReg* unwindRestore =
        dynamic_cast<const CUnwindRestoreReg*>(&a_unwindElt);
    if(unwindRestore)
        {
            CUnwindElement::CopyMembers(a_unwindElt);
            RegId(unwindRestore->RegId());
        }
    else
        {
            DevAssert(FALSE, ("We should not get here %s %d", __FILE__,
                              __LINE__));
        }
}

//-----------------------------------------------------------------------------
// CUnwindRestoreRegNoSp class definition
//-----------------------------------------------------------------------------
CUnwindRestoreRegNoSp::CUnwindRestoreRegNoSp(MEM_POOL* memPool,
                                             const DebugRegId& regId,
                                             const DebugRegId& saveRegId,
                                             UINT offsetFromBegin,
                                             const ElfSection& section)
    :CUnwindRestoreReg(memPool, regId, offsetFromBegin, section)
{
    SaveRegId(saveRegId);
}

CUnwindRestoreRegNoSp::CUnwindRestoreRegNoSp(const CUnwindRestoreReg& a_unwind)
    :CUnwindRestoreReg(a_unwind)
{
    CopyMembers(a_unwind);
}

CUnwindRestoreRegNoSp&
CUnwindRestoreRegNoSp::operator=(const CUnwindRestoreRegNoSp& a_unwind)
{
    if(this != &a_unwind)
        {
            static_cast<void>(CUnwindElement::operator=(a_unwind));
            CopyMembers(a_unwind);
        }
    return *this;
}


CUnwindRestoreRegNoSp::~CUnwindRestoreRegNoSp()
{
}

CUnwindElement*
CUnwindRestoreRegNoSp::Clone(MEM_POOL* a_mem_pool) const
{
    return CXX_NEW(CUnwindRestoreRegNoSp(*this), a_mem_pool);
}

BOOL
CUnwindRestoreRegNoSp::IsOpposite(const CUnwindElement& a_ue) const
{
    const CUnwindSaveRegNoSp* tmp =
        dynamic_cast<const CUnwindSaveRegNoSp*>(&a_ue);
    return tmp && tmp->RegId() == RegId() && tmp->SaveRegId() == SaveRegId();
}

const DebugRegId&
CUnwindRestoreRegNoSp::SaveRegId() const
{
    return m_saveRegId;
}

void
CUnwindRestoreRegNoSp::SaveRegId(const DebugRegId& saveRegId)
{
    m_saveRegId = saveRegId;
}


void
CUnwindRestoreRegNoSp::CopyMembers(const CUnwindElement& a_unwindElt)
{
    const CUnwindRestoreRegNoSp* unwindRestore =
        dynamic_cast<const CUnwindRestoreRegNoSp*>(&a_unwindElt);
    if(unwindRestore)
        {
            CUnwindElement::CopyMembers(a_unwindElt);
            SaveRegId(unwindRestore->SaveRegId());
        }
    else
        {
            DevAssert(FALSE, ("We should not get here %s %d", __FILE__,
                              __LINE__));
        }
}

//-----------------------------------------------------------------------------
// CUnwindCFARelativeOffset class definition
//-----------------------------------------------------------------------------
CUnwindCFARelativeOffset::CUnwindCFARelativeOffset(MEM_POOL* memPool,
                                                   INT cfaOffset,
                                                   UINT offsetFromBegin,
                                                   const ElfSection& section)
    :CUnwindElement(memPool, offsetFromBegin, section)
{
    CfaOffset(cfaOffset);
}
                                                   

CUnwindCFARelativeOffset::CUnwindCFARelativeOffset(const CUnwindCFARelativeOffset& a_unwindCfa)
    :CUnwindElement(a_unwindCfa)
{
    CopyMembers(a_unwindCfa);
}

CUnwindCFARelativeOffset&
CUnwindCFARelativeOffset::operator=(const CUnwindCFARelativeOffset& a_unwind)
{
    if(this != &a_unwind)
        {
            static_cast<void>(CUnwindElement::operator=(a_unwind));
            CopyMembers(a_unwind);
        }
    return *this;
}

CUnwindCFARelativeOffset::~CUnwindCFARelativeOffset()
{
}

Fde
CUnwindCFARelativeOffset::AddFdeInst(Fde fde, ErrorDesc* errorDesc) const
{
    return dwarf_add_fde_inst(fde, DW_CFA_def_cfa_offset,
                              (Dwarf_Unsigned)CfaOffset(), (Dwarf_Unsigned)0,
                              errorDesc);
}

CUnwindElement*
CUnwindCFARelativeOffset::Clone(MEM_POOL* a_mem_pool) const
{
    return CXX_NEW(CUnwindCFARelativeOffset(*this), a_mem_pool);
}

void
CUnwindCFARelativeOffset::Reloc(const CRelocationInfo& relocInfo)
{
    CUnwindElement::Reloc(relocInfo);
    CfaOffset(CfaOffset() + relocInfo.CfaOffset());
}

BOOL
CUnwindCFARelativeOffset::IsSame(const CUnwindElement& ue) const
{
    const CUnwindCFARelativeOffset* tmp =
        dynamic_cast<const CUnwindCFARelativeOffset*>(&ue);
    return tmp  && tmp->CfaOffset() == CfaOffset();
}

BOOL
CUnwindCFARelativeOffset::IsOpposite(const CUnwindElement& ue) const
{
    return FALSE;
}

INT
CUnwindCFARelativeOffset::CfaOffset() const
{
    return m_cfaOffset;
}

void
CUnwindCFARelativeOffset::CfaOffset(INT cfaOffset)
{
    m_cfaOffset = cfaOffset;
}

void
CUnwindCFARelativeOffset::CopyMembers(const CUnwindElement& a_unwindElt)
{
    const CUnwindCFARelativeOffset* unwindCfa =
        dynamic_cast<const CUnwindCFARelativeOffset*>(&a_unwindElt);
    if(unwindCfa)
        {
            CUnwindElement::CopyMembers(a_unwindElt);
            CfaOffset(unwindCfa->CfaOffset());
        }
    else
        {
            DevAssert(FALSE, ("We should not get here %s %d", __FILE__,
                              __LINE__));
        }
}


//-----------------------------------------------------------------------------
// CUnwindCFADefineRegister class definition
//-----------------------------------------------------------------------------
CUnwindCFADefineRegister::CUnwindCFADefineRegister(MEM_POOL* memPool,
                                                   const DebugRegId& regId,
                                                   UINT offsetFromBegin,
                                                   const ElfSection& section)
    :CUnwindElement(memPool, offsetFromBegin, section)
{
    RegId(regId);
}
                                                   

CUnwindCFADefineRegister::CUnwindCFADefineRegister(const CUnwindCFADefineRegister& a_unwindCfa)
    :CUnwindElement(a_unwindCfa)
{
    CopyMembers(a_unwindCfa);
}

CUnwindCFADefineRegister&
CUnwindCFADefineRegister::operator=(const CUnwindCFADefineRegister& a_unwind)
{
    if(this != &a_unwind)
        {
            static_cast<void>(CUnwindElement::operator=(a_unwind));
            CopyMembers(a_unwind);
        }
    return *this;
}

CUnwindCFADefineRegister::~CUnwindCFADefineRegister()
{
}

Fde
CUnwindCFADefineRegister::AddFdeInst(Fde fde, ErrorDesc* errorDesc) const
{
    return dwarf_add_fde_inst(fde, DW_CFA_def_cfa_register,
                              (Dwarf_Unsigned)RegId(), (Dwarf_Unsigned)0,
                              errorDesc);
}

CUnwindElement*
CUnwindCFADefineRegister::Clone(MEM_POOL* a_mem_pool) const
{
    return CXX_NEW(CUnwindCFADefineRegister(*this), a_mem_pool);
}

BOOL
CUnwindCFADefineRegister::IsSame(const CUnwindElement& ue) const
{
    const CUnwindCFADefineRegister* tmp =
        dynamic_cast<const CUnwindCFADefineRegister*>(&ue);
    return tmp  && tmp->RegId() == RegId();
}

BOOL
CUnwindCFADefineRegister::IsOpposite(const CUnwindElement& ue) const
{
    return FALSE;
}

const DebugRegId&
CUnwindCFADefineRegister::RegId() const
{
    return m_regId;
}

void
CUnwindCFADefineRegister::RegId(const DebugRegId& regId)
{
    m_regId = regId;
}

void
CUnwindCFADefineRegister::CopyMembers(const CUnwindElement& a_unwindElt)
{
    const CUnwindCFADefineRegister* unwindCfa =
        dynamic_cast<const CUnwindCFADefineRegister*>(&a_unwindElt);
    if(unwindCfa)
        {
            CUnwindElement::CopyMembers(a_unwindElt);
            RegId(unwindCfa->RegId());
        }
    else
        {
            DevAssert(FALSE, ("We should not get here %s %d", __FILE__,
                              __LINE__));
        }
}

//-----------------------------------------------------------------------------
// CUnwindAsm class definition
//-----------------------------------------------------------------------------
CUnwindAsm::CUnwindAsm(MEM_POOL* memPool, LABEL_IDX start, LABEL_IDX end,
                       UINT offsetFromBegin, const ElfSection& section)
    :CUnwindElement(memPool, offsetFromBegin, section)
{
    StartLabel(start);
    EndLabel(end);
}
                                                   

CUnwindAsm::CUnwindAsm(const CUnwindAsm& a_unwindCfa)
    :CUnwindElement(a_unwindCfa)
{
    CopyMembers(a_unwindCfa);
}

CUnwindAsm&
CUnwindAsm::operator=(const CUnwindAsm& a_unwind)
{
    if(this != &a_unwind)
        {
            static_cast<void>(CUnwindElement::operator=(a_unwind));
            CopyMembers(a_unwind);
        }
    return *this;
}

CUnwindAsm::~CUnwindAsm()
{
}

Fde
CUnwindAsm::AddFdeInst(Fde fde, ErrorDesc* errorDesc) const
{
    return dwarf_add_fde_inst(fde, DW_CFA_ST_relocation,
                              Cg_Dwarf_Symtab_Entry(CGD_LABIDX, StartLabel(),
                                                    Section()),
                              Cg_Dwarf_Symtab_Entry(CGD_LABIDX, EndLabel(),
                                                    Section()),
                              errorDesc);
}

CUnwindElement*
CUnwindAsm::Clone(MEM_POOL* a_mem_pool) const
{
    return CXX_NEW(CUnwindAsm(*this), a_mem_pool);
}

BOOL
CUnwindAsm::IsSame(const CUnwindElement& ue) const
{
    const CUnwindAsm* tmp =
        dynamic_cast<const CUnwindAsm*>(&ue);
    return tmp && tmp->StartLabel() == StartLabel() &&
        tmp->EndLabel() == EndLabel();
}

BOOL
CUnwindAsm::IsOpposite(const CUnwindElement& ue) const
{
    return FALSE;
}

LABEL_IDX
CUnwindAsm::StartLabel() const
{
    return m_startLabel;
}

LABEL_IDX
CUnwindAsm::EndLabel() const
{
    return m_endLabel;
}

void
CUnwindAsm::StartLabel(LABEL_IDX start)
{
    m_startLabel = start;
}

void
CUnwindAsm::EndLabel(LABEL_IDX end)
{
    m_endLabel = end;
}

void
CUnwindAsm::CopyMembers(const CUnwindElement& a_unwindElt)
{
    const CUnwindAsm* unwindCfa =
        dynamic_cast<const CUnwindAsm*>(&a_unwindElt);
    if(unwindCfa)
        {
            CUnwindElement::CopyMembers(a_unwindElt);
            StartLabel(unwindCfa->StartLabel());
            EndLabel(unwindCfa->EndLabel());
        }
    else
        {
            DevAssert(FALSE, ("We should not get here %s %d", __FILE__,
                              __LINE__));
        }
}
