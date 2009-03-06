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
 * @file   targ_cgdwarf_utils.h
 * @author Quentin COLOMBET
 * @date   Wed May 17 14:38:51 2006
 * @brief  This file contains the declaration of some classes that represents
 *         debug information and more particularly unwind element, i.e. debug
 *         frame information.
 * 
 */

#ifndef __TARG_CGDWARF_UTILS_H__
#define __TARG_CGDWARF_UTILS_H__

// System include
#include <libelf.h>   // Needed by libdwarf.h
#include <libdwarf.h> // For Fde definition and dwarf_add_fde_inst usage

// User include
#include "mempool.h"  // Needed for MEM_POOL declaration
#include "symtab_idx.h" // Needed for LABEL_IDX type definition
#include "targ_em_dwarf.h" // Needed for DebugRegId definition

typedef Dwarf_P_Fde Fde;
typedef Dwarf_Error ErrorDesc;
typedef Elf64_Word ElfSection;

// Section used to initialize elf section field
extern const ElfSection g_defaultSection;

// Forward declaration
class CRelocationInfo;

/**
 * @class CUnwindElement
 * This is an abstract class which represents an unwind element.
 */
class CUnwindElement
{
 public:

    /**
     * Constructor.
     * Initialize the members of the object with the given values.
     *
     * @param  memPool [in] Memory pool used to store all memories allocated
     *         for this object.
     * @param  offsetFromBegin Offset of the instruction that is represented by
     *         this unwind element.
     * @param  section Section that will store the unwind element
     *
     * @pre    true
     * @post   Offset() = offsetFromBegin and MemPool() = memPool and
     *         Section() = section
     *
     */
    CUnwindElement(MEM_POOL* memPool, UINT offsetFromBegin = 0,
                   const ElfSection& section = g_defaultSection);

    /**
     * Copy constructor.
     * Initialize the members of the object with the values of the members of
     * the given object
     *
     * @param  a_unwindElt The object to be copied
     *
     * @pre    true
     * @post   Offset() = a_unwindElt.Offset() and MemPool() =
     *         a_unwindElt.MemPool() and Section() = a_unwindElt.Section()
     *
     * @warning When using this constructor, the mempool of a_unwindElt is
     *          used, i.e. you cannot use this constructor to change the scope
     *          (in terms of mempool) of the created object.
     */
    CUnwindElement(const CUnwindElement& a_unwindElt);

    /**
     * operator=.
     * Set the  members values of this object to the members values
     * of the given one
     *
     * @param  a_unwindElt To be copied
     *
     * @pre    true
     * @post   Offset() = a_unwindElt.Offset() and MemPool() =
     *         a_unwindElt.MemPool() and Section() = a_unwindElt.Section()
     *
     * @return The modified object, i.e. the lhs of the equal operation
     *
     * @warning The same limitation of copy constructor applied with this
     *          operator
     */
    CUnwindElement&
        operator=(const CUnwindElement& a_unwindElt);

    /**
     * Destructor. 
     *
     * @pre    true
     * @post   'this' is not a valid object anymore
     *
     */
    virtual
        ~CUnwindElement();

    /**
     * Add the related debug frame information to the given frame descriptor
     * entry.
     *
     * @param  fde The frame descriptor entry which will receive the related
     *         debug frame instruction
     * @param  errorDesc A pointer to a dwarf error. Unless it is
     *         set to null, this variable will contains the error description
     *         (@see libdwarf documentation for more details)
     *
     * @pre    fde has been initialized
     * @post   if fde = DW_DLV_BADADDR then errorDesc contains the error
     *         description and must be freed using the dwarf_dealloc function
     *         else fde contains the dwarf instruction of the object endif.
     *
     * @return The given fde on success, and DW_DLV_BADADDR on error
     */
    virtual Fde
        AddFdeInst(Fde fde, ErrorDesc* errorDesc) const = 0;

    /**
     * Check whether two unwind elements are the same.
     * Unwind element are considered as 'same' when all their arguments are
     * equal except the offset one, since it represents the position of the
     * unwind element in the program.
     *
     * @param  ue The unwind element to be compared to
     *
     * @pre    true
     * @post   true
     *
     * @return TRUE if this and ue can be considered as the same unwind
     *         element, FALSE otherwise
     */
    virtual BOOL
        IsSame(const CUnwindElement& ue) const = 0;

    /**
     * Check whether two unwind elements are opposite.
     * Unwind element are considered as 'opposite' when one represents the
     * reverse operation of the second (e.g. save vs restore).
     *
     * @param  ue The unwind element to be compared to
     *
     * @pre    true
     * @post   true
     *
     * @return TRUE if this and ue can be considered as opposite unwind
     *         element, FALSE otherwise
     */
    virtual BOOL
        IsOpposite(const CUnwindElement& ue) const = 0;

    /**
     * Getter for the m_offset member.
     *
     * @pre    true
     * @post   true
     *
     * @return The value of the m_offset member
     */
    UINT
        Offset() const;

    /**
     * Getter for the m_section member.
     *
     * @pre    true
     * @post   true
     *
     * @return The value of the m_section member
     */
    const ElfSection&
        Section() const;

    /**
     * Getter for the m_memPool member.
     *
     * @pre    true
     * @post   true
     *
     * @return The m_memPool member
     */
    MEM_POOL*
        MemPool() const;

    /**
     * Create a new unwind element with the current unwind element.
     * The newly created unwind element as the same type as this and as all
     * its members values initialized to the values of this. The created object
     * is totally independant, i.e. you can destroy this without altering the
     * values of the returned object.
     *
     * @param  a_mem_pool The mempool used to allocate the returned object
     *
     * @pre    true
     * @post   result.IsSame(this) and result.Offset() = Offset()
     *
     * @return The created object.
     *
     * @warning the created object is dynamically allocated and must be freed
     *          using CXX_DELETE on the given a_mem_pool
     *
     * @remarks For all inherited classes the returned object must be
     *          independant. I.e. modifying / destroying this must not affect
     *          the returned object
     */
    virtual CUnwindElement*
        Clone(MEM_POOL* a_mem_pool) const = 0;

    /**
     * Relocation method.
     * This method is used to update the information of the current object
     * that are context depend like the offset of the instruction in the
     * program
     *
     * @param  a_relocInfo Contains the relocation information used to update
     *         this
     *
     * @pre    true
     * @post   Offset() = self@pre.Offset() + a_relocInfo.Offset() and
     *         Section() = a_relocInfo.Section()
     *
     */
    virtual void
        Reloc(const CRelocationInfo& a_relocInfo);

 protected:

    /**
     * Setter for m_offset member.
     * Set m_offset member with the given offset
     *
     * @param  offsetFromBegin Offset to be assigned to m_offset member
     *
     * @pre    true
     * @post   Offset() = offsetFromBegin
     *
     */
    void
        Offset(UINT offsetFromBegin);

    /**
     * Setter for m_section member.
     * Set m_section member with the given section
     *
     * @param  section Section to be assigned to m_offset member
     *
     * @pre    true
     * @post   Offset() = offsetFromBegin
     *
     */
    void
        Section(const ElfSection& section);

    /**
     * Setter for m_memPool member.
     * Set m_memPool member with the given mempool
     *
     * @param  memPool [in] Mempool to be assigned to m_memPool member
     *
     * @pre    true
     * @post   MemPool() = memPool
     *
     */
    void
        MemPool(MEM_POOL* memPool);

    /**
     * Copy members values of given unwind element in this.
     * The purpose of this method is to set all members values of this to
     * the members values of given unwind element
     *
     * @param  a_unwindElt Unwind element to be copied
     *
     * @pre    true
     * @post   Offset() = a_unwindElt.Offset() and MemPool() =
     *         a_unwindElt.MemPool()
     *
     */
    virtual void
        CopyMembers(const CUnwindElement& a_unwindElt);

    /**
     * Offset member.
     * This member represents the global position of the unwind element in the
     * related function.
     */
    UINT m_offset;

    /**
     * Section member.
     * This member represents the elf section where this unwind element will be
     * emitted
     */
    ElfSection m_section;

    /**
     * Mempool member.
     * Memory pool used to allocate (if any) all memory for the current object
     */
    MEM_POOL* m_memPool;
};

/**
 * @class CUnwindSaveReg
 * This class represents debug information of an operation of register saving
 * on the stack
 */
class CUnwindSaveReg : public CUnwindElement
{
 public:

    /**
     * Constructor.
     * Initialze all members values to the given values.
     *
     * @param  memPool [in] The memory pool that will be used for object
     *         internal memory allcoation (if any)
     * @param  regId The debug register identifier of the saved register
     * @param  cfaOffset CFA offset used to store this saved register
     * @param  offsetFromBegin The operation offset from the begin of the
     *         function
     * @param  section Section that will store the unwind element
     *
     * @pre    true
     * @post   MemPool() = memPool and RegId() = regId and CfaOffset() =
     *         cfaOffset and Offset() = offsetFromBegin
     */
    CUnwindSaveReg(MEM_POOL* memPool, const DebugRegId& regId, INT cfaOffset,
                   UINT offsetFromBegin = 0,
                   const ElfSection& section = g_defaultSection);

    /**
     * Copy constructor.
     * Initialize all members values to given object members values.
     *
     * @param  a_unwind Object to be copied
     *
     * @pre    true
     * @post   MemPool() = a_unwind.MemPool() and RegId() = a_unwind.RegId() and
     *         CfaOffset() = a_unwind.CfaOffset() and Offset() =
     *         a_unwind.Offset()
     */
    CUnwindSaveReg(const CUnwindSaveReg& a_unwind);

    /**
     * operator=.
     * Set the members values of this object to the members values
     * of the given one
     *
     * @param  a_unwind To be copied 
     *
     * @pre    true
     * @post   MemPool() = a_unwind.MemPool() and RegId() = a_unwind.RegId() and
     *         CfaOffset() = a_unwind.CfaOffset() and Offset() =
     *         a_unwind.Offset()
     */
    CUnwindSaveReg&
        operator=(const CUnwindSaveReg& a_unwind);

    /**
     * Destructor.
     *
     * @pre    true
     * @post   'this' is not a valid object anymore
     */
    virtual
        ~CUnwindSaveReg();

    /**
     * Add a saved register debug frame information (DW_CFA_offset or
     * equivalent) in the given fde. The saved register is set to RegId() and
     * the related CFA offset to CfaOffset().
     * @see CUnwindElement::AddFdeInst for a complete description
     */
    virtual Fde
        AddFdeInst(Fde fde, ErrorDesc* errorDesc) const;

    /**
     * @see CUnwindElement::Clone for a complete description
     */
    virtual CUnwindElement*
        Clone(MEM_POOL* a_mem_pool) const;

    /**
     * Relocation method.
     * @see CUnwindElement::Reloc for complete description
     *
     * @pre    Same as CUnwindElement::Reloc pre-condition
     * @post   CUnwindElement::Reloc post-condition are true and CfaOffset() = 
     *         self@pre.CfaOffset() + a_relocInfo.CfaOffset()
     */
    virtual void
        Reloc(const CRelocationInfo& a_relocInfo);

    /**
     * @see CUnwindElement::IsSame for a complete description
     */
    virtual BOOL
        IsSame(const CUnwindElement& ue) const;

    /**
     * @see CUnwindElement::IsOpposite for a complete description
     *
     * @remarks For this kind of unwind element, an opposite one is a restored
     *          register unwind element with the same register id.
     */
    virtual BOOL
        IsOpposite(const CUnwindElement& ue) const;

    /**
     * Getter for m_regId member.
     *
     * @pre    true
     * @post   true
     *
     * @return A constant reference to m_regId member
     */
    const DebugRegId&
        RegId() const;

    /**
     * Getter for m_cfaOffset member.
     *
     * @pre    true
     * @post   true
     *
     * @return The value of m_cfaOffset member
     */
    INT
        CfaOffset() const;

 protected:

    /**
     * Setter for m_regId member.
     * Set m_regId member with the given regId
     *
     * @param  regId Debug register identifier to be assigned to m_regId member
     *
     * @pre    true
     * @post   RegId() = regId
     *
     */
    void
        RegId(const DebugRegId& regId);

    /**
     * Setter for m_cfaOffset member.
     * Set m_cfaOffset member with the given cfaOffset
     *
     * @param  cfaOffset CFA offset to be assigned to m_cfaOffset member
     *
     * @pre    true
     * @post   CfaOffset() = cfaOffset
     *
     */
    void
        CfaOffset(INT cfaOffset);

    /**
     * @see CUnwindElement::CopyMembers for a complete description
     *
     * @pre    Same pre-conditions as CUnwindElement::CopyMembers
     * @post   all CUnwindElement::CopyMembers post-conditions are true and
     *         CfaOffset() = a_unwindElt.CfaOffset() and RegId() =
     *         a_unwindElt.RegId()
     *
     */
    virtual void
        CopyMembers(const CUnwindElement& a_unwindElt);

    /**
     * Register identifier member.
     * This member represents the debug register identifier of the saved
     * register of the instruction represented by this unwind element
     */
    DebugRegId m_regId;

    /**
     * CFA offset member.
     * This member represents the CFA offset used to store the saved register
     * of the instruction represented by this unwind element
     */
    INT m_cfaOffset;
};

/**
 * @class CUnwindSaveRegNoSp
 * This class represents debug information of an operation of register saving
 * in another register. 
 */
class CUnwindSaveRegNoSp : public CUnwindElement
{
 public:

    /**
     * Constructor.
     * Initialze all members values to the given values.
     *
     * @param  memPool [in] The memory pool that will be used for object
     *         internal memory allcoation (if any)
     * @param  regId The debug register identifier of the saved register
     * @param  saveRegId The debug register identifier of the destination
     *         register
     * @param  offsetFromBegin The operation offset from the begin of the
     *         function
     * @param  section Section that will store the unwind element
     *
     * @pre    true
     * @post   MemPool() = memPool and RegId() = regId and SaveRegId() =
     *         saveRegId and Offset() = offsetFromBegin
     */
    CUnwindSaveRegNoSp(MEM_POOL* memPool, const DebugRegId& regId,
                       const DebugRegId& saveRegId, UINT offsetFromBegin = 0,
                       const ElfSection& section = g_defaultSection);

   /**
     * Copy constructor.
     * Initialize all members values to given object members values.
     *
     * @param  a_unwind Object to be copied
     *
     * @pre    true
     * @post   MemPool() = a_unwind.MemPool() and RegId() = a_unwind.RegId() and
     *         SaveRegId() = a_unwind.SaveRegId() and Offset() =
     *         a_unwind.Offset()
     */
    CUnwindSaveRegNoSp(const CUnwindSaveRegNoSp& a_unwind);

   /**
     * operator=.
     * Set the members values of this object to the members values
     * of the given one
     *
     * @param  a_unwind To be copied 
     *
     * @pre    true
     * @post   MemPool() = a_unwind.MemPool() and RegId() = a_unwind.RegId() and
     *         SaveRegId() = a_unwind.SaveRegId() and Offset() =
     *         a_unwind.Offset()
     */
    CUnwindSaveRegNoSp&
        operator=(const CUnwindSaveRegNoSp& a_unwind);

    /**
     * Destructor.
     *
     * @pre    true
     * @post   'this' is not a valid object anymore
     */
    virtual
        ~CUnwindSaveRegNoSp();

    /**
     * Add a saved register debug frame information (DW_CFA_register) in the
     * given fde. The saved register is set to RegId() and the destination
     * register to SaveRegId().
     * @see CUnwindElement::AddFdeInst for a complete description
     */
    virtual Fde
        AddFdeInst(Fde fde, ErrorDesc* errorDesc) const;

    /**
     * @see CUnwindElement::Clone for a complete description
     */
    virtual CUnwindElement*
        Clone(MEM_POOL* a_mem_pool) const;

    /**
     * @see CUnwindElement::IsSame for a complete description
     */
    virtual BOOL
        IsSame(const CUnwindElement& ue) const;

    /**
     * @see CUnwindElement::IsOpposite for a complete description
     *
     * @remarks For this kind of unwind element, an opposite one is a restored
     *          register unwind element with same register ids.
     */
    virtual BOOL
        IsOpposite(const CUnwindElement& ue) const;

    /**
     * Getter for m_saveRegId member.
     *
     * @pre    true
     * @post   true
     *
     * @return A constant reference to m_saveRegId member
     */
    const DebugRegId&
        SaveRegId() const;

    /**
     * Getter for m_regId member.
     *
     * @pre    true
     * @post   true
     *
     * @return A constant reference to m_regId member
     */
    const DebugRegId&
        RegId() const;

 protected:

    /**
     * Setter for m_saveRegId member.
     * Set m_saveRegId member with the given saveRegId
     *
     * @param  saveRegId Debug register identifier to be assigned to
     *         m_saveRegId member
     *
     * @pre    true
     * @post   m_saveRegId = saveRegId
     *
     */
    void
        SaveRegId(const DebugRegId& saveRegId);

    /**
     * Setter for m_regId member.
     * Set m_regId member with the given regId
     *
     * @param  regId Debug register identifier to be assigned to m_regId member
     *
     * @pre    true
     * @post   m_regId = regId
     *
     */
    void
        RegId(const DebugRegId& regId);

    /**
     * @see CUnwindElement::CopyMembers for a complete description
     *
     * @pre    Same pre-conditions as CUnwindElement::CopyMembers
     * @post   all CUnwindElement::CopyMembers post-conditions are true and
     *         SaveRegId() = a_unwindElt.SaveRegId() and RegId() =
     *         a_unwindElt.RegId()
     *
     */
    virtual void
        CopyMembers(const CUnwindElement& a_unwindElt);

    /**
     * Register identifier member.
     * This member represents the debug register identifier of the saved
     * register of the instruction represented by this unwind element
     */
    DebugRegId m_regId;

    /**
     * Save register identifier member.
     * This member represents the debug register identifier of the register
     * used to store the saved register of the instruction represented by this
     * unwind element
     */
    DebugRegId m_saveRegId;
};

/**
 * @class CUnwindRestoreReg
 * This class represents debug information of an operation of register
 * restoring from the stack
 */
class CUnwindRestoreReg : public CUnwindElement
{
 public:

   /**
     * Constructor.
     * Initialze all members values to the given values.
     *
     * @param  memPool [in] The memory pool that will be used for object
     *         internal memory allcoation (if any)
     * @param  regId The debug register identifier of the restored register
     * @param  offsetFromBegin The operation offset from the begin of the
     *         function
     * @param  section Section that will store the unwind element
     *
     * @pre    true
     * @post   MemPool() = memPool and RegId() = regId and Offset() =
     *         offsetFromBegin
     */
    CUnwindRestoreReg(MEM_POOL* memPool, const DebugRegId& regId,
                      UINT offsetFromBegin = 0,
                      const ElfSection& section = g_defaultSection);

    /**
     * Copy constructor.
     * Initialize all members values to given object members values.
     *
     * @param  a_unwind Object to be copied
     *
     * @pre    true
     * @post   MemPool() = a_unwind.MemPool() and RegId() = a_unwind.RegId() and
     *         Offset() = a_unwind.Offset()
     */
    CUnwindRestoreReg(const CUnwindRestoreReg& a_unwind);

    /**
     * operator=.
     * Set the members values of this object to the members values
     * of the given one
     *
     * @param  a_unwind To be copied 
     *
     * @pre    true
     * @post   MemPool() = a_unwind.MemPool() and RegId() = a_unwind.RegId() and
     *         and Offset() = a_unwind.Offset()
     */
    CUnwindRestoreReg&
        operator=(const CUnwindRestoreReg& a_unwind);

    /**
     * Destructor.
     *
     * @pre    true
     * @post   'this' is not a valid object anymore
     */
    virtual
        ~CUnwindRestoreReg();

    /**
     * Add a restored register debug frame information (DW_CFA_restore or
     * equivalent) in the given fde. The restored register is set to RegId()
     * @see CUnwindElement::AddFdeInst for a complete description
     */
    virtual Fde
        AddFdeInst(Fde fde, ErrorDesc* errorDesc) const;

    /**
     * @see CUnwindElement::Clone for a complete description
     */
    virtual CUnwindElement*
        Clone(MEM_POOL* a_mem_pool) const;

    /**
     * @see CUnwindElement::IsSame for a complete description
     */
    virtual BOOL
        IsSame(const CUnwindElement& ue) const;

    /**
     * @see CUnwindElement::IsOpposite for a complete description
     *
     * @remarks For this kind of unwind element, an opposite one is a saved
     *          register unwind element with the same register id.
     */
    virtual BOOL
        IsOpposite(const CUnwindElement& ue) const;

    /**
     * Getter for m_regId member.
     *
     * @pre    true
     * @post   true
     *
     * @return A constant reference to m_regId member
     */
    const DebugRegId&
        RegId() const;

 protected:

    /**
     * Setter for m_regId member.
     * Set m_regId member with the given regId
     *
     * @param  regId Debug register identifier to be assigned to m_regId member
     *
     * @pre    true
     * @post   RegId() = regId
     *
     */
    void
        RegId(const DebugRegId& regId);

    /**
     * @see CUnwindElement::CopyMembers for a complete description
     *
     * @pre    Same pre-conditions as CUnwindElement::CopyMembers
     * @post   all CUnwindElement::CopyMembers post-conditions are true and
     *         RegId() = a_unwindElt.RegId()
     */
    virtual void
        CopyMembers(const CUnwindElement& a_unwindElt);

    /**
     * Register identifier member.
     * This member represents the debug register identifier of the restored
     * register of the instruction represented by this unwind element
     */
    DebugRegId m_regId;
};

/**
 * @class CUnwindRestoreRegNoSp
 * This class represents debug information of an operation of register
 * restoring from the another register
 */
class CUnwindRestoreRegNoSp : public CUnwindRestoreReg
{
 public:

    /**
     * Constructor.
     * Initialze all members values to the given values.
     *
     * @param  memPool [in] The memory pool that will be used for object
     *         internal memory allcoation (if any)
     * @param  regId The debug register identifier of the restored register
     * @param  saveRegId The debug register identifier of the source
     *         register
     * @param  offsetFromBegin The operation offset from the begin of the
     *         function
     * @param  section Section that will store the unwind element
     *
     * @pre    true
     * @post   MemPool() = memPool and RegId() = regId and SaveRegId() =
     *         saveRegId and Offset() = offsetFromBegin
     */
    CUnwindRestoreRegNoSp(MEM_POOL* memPool, const DebugRegId& regId,
                          const DebugRegId& saveRegId, UINT offsetFromBegin = 0,
                          const ElfSection& section = g_defaultSection);

    /**
     * Copy constructor.
     * Initialize all members values to given object members values.
     *
     * @param  a_unwindRestoreNoSp Object to be copied
     *
     * @pre    true
     * @post   MemPool() = a_unwindRestoreNoSp.MemPool() and RegId() =
     *         a_unwindRestoreNoSp.RegId() and SaveRegId() =
     *         a_unwindRestoreNoSp.SaveRegId() and Offset() =
     *         a_unwindRestoreNoSp.Offset()
     */
    CUnwindRestoreRegNoSp(const CUnwindRestoreReg& a_unwindRestoreNoSp);

    /**
     * operator=.
     * Set the members values of this object to the members values
     * of the given one
     *
     * @param  a_unwindRestoreNoSp To be copied 
     *
     * @pre    true
     * @post   MemPool() = a_unwindRestoreNoSp.MemPool() and RegId() =
     *         a_unwindRestoreNoSp.RegId() and SaveRegId() =
     *         a_unwindRestoreNoSp.SaveRegId() and Offset() =
     *         a_unwindRestoreNoSp.Offset()
     */
    CUnwindRestoreRegNoSp&
        operator=(const CUnwindRestoreRegNoSp& a_unwindRestoreNoSp);

    /**
     * Destructor.
     *
     * @pre    true
     * @post   'this' is not a valid object anymore
     */
    virtual
        ~CUnwindRestoreRegNoSp();

    /**
     * @see CUnwindRestoreReg::Clone for a complete description
     */
    virtual CUnwindElement*
        Clone(MEM_POOL* a_mem_pool) const;

    /**
     * @see CUnwindRestoreReg::IsOpposite for a complete description
     *
     * @remarks For this kind of unwind element, an opposite one is a saved
     *          register unwind element with same register ids.
     */
    virtual BOOL
        IsOpposite(const CUnwindElement& ue) const;

    /**
     * Getter for m_saveRegId member.
     *
     * @pre    true
     * @post   true
     *
     * @return A constant reference to m_saveRegId member
     */
    const DebugRegId&
        SaveRegId() const;

 protected:

    /**
     * Setter for m_saveRegId member.
     * Set m_saveRegId member with the given saveRegId
     *
     * @param  saveRegId Debug register identifier to be assigned to
     *         m_saveRegId member
     *
     * @pre    true
     * @post   m_saveRegId = saveRegId
     *
     */
    void
        SaveRegId(const DebugRegId& saveRegId);

    /**
     * @see CUnwindRestoreReg::CopyMembers for a complete description
     *
     * @pre    Same pre-conditions as CUnwindRestoreReg::CopyMembers
     * @post   all CUnwindElement::CopyMembers post-conditions are true and
     *         SaveRegId() = a_unwindElt.SaveRegId()
     *
     */
    virtual void
        CopyMembers(const CUnwindElement& a_unwindElt);

    /**
     * Save register identifier member.
     * This member represents the debug register identifier of the register
     * used to store the saved register of the instruction represented by this
     * unwind element
     */
    DebugRegId m_saveRegId;
};

/**
 * @class CUnwindCFARelativeOffset
 * This class represents debug information of an operation that modified the
 * CFA offset
 */
class CUnwindCFARelativeOffset : public CUnwindElement
{
 public:

    /**
     * Constructor.
     * Initialze all members values to the given values.
     *
     * @param  memPool [in] The memory pool that will be used for object
     *         internal memory allcoation (if any)
     * @param  cfaOffset CFA offset
     * @param  offsetFromBegin The operation offset from the begin of the
     *         function
     * @param  section Section that will store the unwind element
     *
     * @pre    true
     * @post   MemPool() = memPool and and CfaOffset() = cfaOffset and
     *         Offset() = offsetFromBegin
     */
    CUnwindCFARelativeOffset(MEM_POOL* memPool, INT cfaOffset,
                             UINT offsetFromBegin = 0,
                             const ElfSection& section = g_defaultSection);

    /**
     * Copy constructor.
     * Initialize all members values to given object members values.
     *
     * @param  a_unwindCfa Object to be copied
     *
     * @pre    true
     * @post   MemPool() = a_unwindCfa.MemPool() and Offset() =
     *         a_unwindCfa.Offset() and CfaOffset() = a_unwindCfa.CfaOffset()
     */
    CUnwindCFARelativeOffset(const CUnwindCFARelativeOffset& a_unwindCfa);

    /**
     * operator=.
     * Set the members values of this object to the members values
     * of the given one
     *
     * @param  a_unwindCfa To be copied 
     *
     * @pre    true
     * @post   MemPool() = a_unwindCfa.MemPool() and Offset() =
     *         a_unwindCfa.Offset() and CfaOffset() = a_unwindCfa.CfaOffset()
     */
    CUnwindCFARelativeOffset&
        operator=(const CUnwindCFARelativeOffset& a_unwindCfa);

    /**
     * Destructor.
     *
     * @pre    true
     * @post   'this' is not a valid object anymore
     */
    virtual
        ~CUnwindCFARelativeOffset();

    /**
     * Add a definition of CFA offset debug frame information
     * (DW_CFA_def_cfa_offset or equivalent) in the given fde. The defined CFA
     * is set to CfaOffset()
     * @see CUnwindElement::AddFdeInst for a complete description
     */
    virtual Fde
        AddFdeInst(Fde fde, ErrorDesc* errorDesc) const;

    /**
     * @see CUnwindElement::Clone for a complete description
     */
    virtual CUnwindElement*
        Clone(MEM_POOL* a_mem_pool) const;

    /**
     * Relocation method.
     * @see CUnwindElement::Reloc for complete description
     *
     * @pre    Same as CUnwindElement::Reloc pre-condition
     * @post   CUnwindElement::Reloc post-condition are true and CfaOffset() = 
     *         self@pre.CfaOffset() + a_relocInfo.CfaOffset()
     */
    virtual void
        Reloc(const CRelocationInfo& a_relocInfo);

    /**
     * @see CUnwindElement::IsSame for a complete description
     */
    virtual BOOL
        IsSame(const CUnwindElement& ue) const;

    /**
     * @see CUnwindElement::IsOpposite for a complete description
     *
     * @remarks For this kind of unwind element, we considered that it has not
     *          any opposite
     */
    virtual BOOL
        IsOpposite(const CUnwindElement& ue) const;

    /**
     * Getter for m_cfaOffset member.
     *
     * @pre    true
     * @post   true
     *
     * @return The value of m_cfaOffset member
     */
    INT
        CfaOffset() const;

 protected:

    /**
     * Setter for m_cfaOffset member.
     * Set m_cfaOffset member with the given cfaOffset
     *
     * @param  cfaOffset CFA offset to be assigned to m_cfaOffset member
     *
     * @pre    true
     * @post   CfaOffset() = cfaOffset
     *
     */
    void
        CfaOffset(INT cfaOffset);

    /**
     * @see CUnwindElement::CopyMembers for a complete description
     *
     * @pre    Same pre-conditions as CUnwindElement::CopyMembers
     * @post   all CUnwindElement::CopyMembers post-conditions are true and
     *         CfaOffset() = a_unwindElt.CfaOffset()
     *
     */
    virtual void
        CopyMembers(const CUnwindElement& a_unwindElt);

    /**
     * CFA offset member.
     * This member represents the CFA offset definition of the instruction
     * represented by this unwind element
     */
    INT m_cfaOffset;
};

/**
 * @class CUnwindCFADefineRegister
 * This class represents debug information of an operation that modified the
 * CFA register
 */
class CUnwindCFADefineRegister : public CUnwindElement
{
 public:

    /**
     * Constructor.
     * Initialze all members values to the given values.
     *
     * @param  memPool [in] The memory pool that will be used for object
     *         internal memory allcoation (if any)
     * @param  regId Register identifier of the new CFA register
     * @param  offsetFromBegin The operation offset from the begin of the
     *         function
     * @param  section Section that will store the unwind element
     *
     * @pre    true
     * @post   MemPool() = memPool and and RegId() = regId and
     *         Offset() = offsetFromBegin
     */
    CUnwindCFADefineRegister(MEM_POOL* memPool, const DebugRegId& regId,
                             UINT offsetFromBegin = 0,
                             const ElfSection& section = g_defaultSection);

    /**
     * Copy constructor.
     * Initialize all members values to given object members values.
     *
     * @param  a_unwindCfa Object to be copied
     *
     * @pre    true
     * @post   MemPool() = a_unwindCfa.MemPool() and Offset() =
     *         a_unwindCfa.Offset() and RegId() = a_unwindCfa.RegId()
     */
    CUnwindCFADefineRegister(const CUnwindCFADefineRegister& a_unwindCfa);

    /**
     * operator=.
     * Set the members values of this object to the members values
     * of the given one
     *
     * @param  a_unwindCfa To be copied 
     *
     * @pre    true
     * @post   MemPool() = a_unwindCfa.MemPool() and Offset() =
     *         a_unwindCfa.Offset() and RegId() = a_unwindCfa.RegId()
     */
    CUnwindCFADefineRegister&
        operator=(const CUnwindCFADefineRegister& a_unwindCfa);

    /**
     * Destructor.
     *
     * @pre    true
     * @post   'this' is not a valid object anymore
     */
    virtual
        ~CUnwindCFADefineRegister();

    /**
     * Add a definition of CFA register debug frame information
     * (DW_CFA_def_cfa_register or equivalent) in the given fde. The defined CFA
     * is set to RegId()
     * @see CUnwindElement::AddFdeInst for a complete description
     */
    virtual Fde
        AddFdeInst(Fde fde, ErrorDesc* errorDesc) const;

    /**
     * @see CUnwindElement::Clone for a complete description
     */
    virtual CUnwindElement*
        Clone(MEM_POOL* a_mem_pool) const;

    /**
     * @see CUnwindElement::IsSame for a complete description
     */
    virtual BOOL
        IsSame(const CUnwindElement& ue) const;

    /**
     * @see CUnwindElement::IsOpposite for a complete description
     *
     * @remarks For this kind of unwind element, we considered that it has not
     *          any opposite
     */
    virtual BOOL
        IsOpposite(const CUnwindElement& ue) const;

    /**
     * Getter for m_regId member.
     *
     * @pre    true
     * @post   true
     *
     * @return A constant reference to m_regId member
     */
    const DebugRegId&
        RegId() const;

 protected:

    /**
     * Setter for m_regId member.
     * Set m_regId member with the given regId
     *
     * @param  regId CFA register identifier to be assigned to m_regId member
     *
     * @pre    true
     * @post   RegId() = regId
     *
     */
    void
        RegId(const DebugRegId& regId);

    /**
     * @see CUnwindElement::CopyMembers for a complete description
     *
     * @pre    Same pre-conditions as CUnwindElement::CopyMembers
     * @post   all CUnwindElement::CopyMembers post-conditions are true and
     *         RegId() = a_unwindElt.RegId()
     *
     */
    virtual void
        CopyMembers(const CUnwindElement& a_unwindElt);

    /**
     * CFA register member.
     * This member represents the CFA register definition of the instruction
     * represented by this unwind element
     */
    DebugRegId m_regId;
};

/**
 * @class CUnwindAsm
 * This class represents an asm operation that might modify debug information.
 * At least it modify the advance loc.
 */
class CUnwindAsm : public CUnwindElement
{
 public:

    /**
     * Constructor.
     * Initialze all members values to the given values.
     *
     * @param  memPool [in] The memory pool that will be used for object
     *         internal memory allcoation (if any)
     * @param  start Identifier of the label placed just before this unwind
     *         element
     * @param  end Identifier of the label placed just after this unwind
     *         element
     * @param  offsetFromBegin The operation offset from the begin of the
     *         function
     * @param  section Section that will store the unwind element
     *
     * @pre    true
     * @post   MemPool() = memPool and and StartLabel() = start and
     *         EndLabel() = end and Offset() = offsetFromBegin
     */
    CUnwindAsm(MEM_POOL* memPool, LABEL_IDX start = 0, LABEL_IDX end = 0,
               UINT offsetFromBegin = 0,
               const ElfSection& section = g_defaultSection);

    /**
     * Copy constructor.
     * Initialize all members values to given object members values.
     *
     * @param  a_unwindCfa Object to be copied
     *
     * @pre    true
     * @post   MemPool() = a_unwindCfa.MemPool() and Offset() =
     *         a_unwindCfa.Offset() and StartLabel() = a_unwindCfa.StartLabel()
     *         and EndLabel() = a_unwindCfa.EndLabel()
     */
    CUnwindAsm(const CUnwindAsm& a_unwindCfa);

    /**
     * operator=.
     * Set the members values of this object to the members values
     * of the given one
     *
     * @param  a_unwindCfa To be copied 
     *
     * @pre    true
     * @post   MemPool() = a_unwindCfa.MemPool() and Offset() =
     *         a_unwindCfa.Offset() and StartLabel() = a_unwindCfa.StartLabel()
     *         and EndLabel() = a_unwindCfa.EndLabel()
     */
    CUnwindAsm&
        operator=(const CUnwindAsm& a_unwindCfa);

    /**
     * Destructor.
     *
     * @pre    true
     * @post   'this' is not a valid object anymore
     */
    virtual
        ~CUnwindAsm();

    /**
     * Add a definition of CFA advance location. This location aims to restore
     * the offset defined globaly between the operations.
     * @see CUnwindElement::AddFdeInst for a complete description
     */
    virtual Fde
        AddFdeInst(Fde fde, ErrorDesc* errorDesc) const;

    /**
     * @see CUnwindElement::Clone for a complete description
     */
    virtual CUnwindElement*
        Clone(MEM_POOL* a_mem_pool) const;

    /**
     * @see CUnwindElement::IsSame for a complete description
     */
    virtual BOOL
        IsSame(const CUnwindElement& ue) const;

    /**
     * @see CUnwindElement::IsOpposite for a complete description
     *
     * @remarks For this kind of unwind element, we considered that it has not
     *          any opposite
     */
    virtual BOOL
        IsOpposite(const CUnwindElement& ue) const;

    /**
     * Getter for m_startLabel member.
     *
     * @pre    true
     * @post   true
     *
     * @return The value of m_startLabel member
     */
    LABEL_IDX
        StartLabel() const;

    /**
     * Getter for m_endLabel member.
     *
     * @pre    true
     * @post   true
     *
     * @return The value of m_endLabel member
     */
    LABEL_IDX
        EndLabel() const;

    /**
     * Setter for m_startLabel member.
     * Set m_startLabel member with the given start
     *
     * @param  start Label identifier to be assigned to m_startLabel member
     *
     * @pre    true
     * @post   StartLabel() = start
     *
     */
    void
        StartLabel(LABEL_IDX start);

    /**
     * Setter for m_endLabel member.
     * Set m_endLabel member with the given end
     *
     * @param  end Label identifier to be assigned to m_endLabel member
     *
     * @pre    true
     * @post   EndLabel() = end
     *
     */
    void
        EndLabel(LABEL_IDX end);

 protected:

    /**
     * @see CUnwindElement::CopyMembers for a complete description
     *
     * @pre    Same pre-conditions as CUnwindElement::CopyMembers
     * @post   all CUnwindElement::CopyMembers post-conditions are true and
     *         StartLabel() = a_unwindElt.StartLabel() and EndLabel() =
     *         a_unwindElt.EndLabel()
     *
     */
    virtual void
        CopyMembers(const CUnwindElement& a_unwindElt);

    /**
     * Identifier of start label member.
     * This member represents the identifier of the label that is directly
     * before related asm statement
     */
    LABEL_IDX m_startLabel;

    /**
     * Identifier of end label member.
     * This member represents the identifier of the label that is directly
     * after related asm statement
     */
    LABEL_IDX m_endLabel;
};

/**
 * @class CRelocationInfo
 * This class represents a relocation information. This information is used to
 * relocate the unwind elements for instance.
 *
 * @remarks If you create a new unwind element that need a more specific
 *          relocation information, you should create a new class that inherit
 *          from this one. In that case, we must add a not inlined virtual
 *          destructor.
 */
class CRelocationInfo
{
 public:

    /**
     * Default constructor.
     * Initialize members values with the given parameters
     *
     * @param  opOffset Operation offset
     * @param  cfaOffset CFA offset
     *
     * @pre    true
     * @post   CfaOffset() = cfaOffset and OpOffset() = opOffset()
     *
     */
    CRelocationInfo(INT opOffset = 0, INT cfaOffset = 0,
                    const ElfSection& section = g_defaultSection)
    {
        OpOffset(opOffset);
        CfaOffset(cfaOffset);
        Section(section);
    }

    /**
     * Getter for m_cfaOffset member.
     *
     * @pre    true
     * @post   true
     *
     * @return The value of m_cfaOffset member
     */
    INT
        CfaOffset() const
    {
        return m_cfaOffset;
    }

    /**
     * Getter for m_opOffset member.
     *
     * @pre    true
     * @post   true
     *
     * @return The value of m_opOffset
     */
    INT
        OpOffset() const
    {
        return m_opOffset;
    }

    /**
     * Getter for m_section member.
     *
     * @pre    true
     * @post   true
     *
     * @return The value of m_section
     */
    const ElfSection&
        Section() const
    {
        return m_section;
    }

 protected:

    /**
     * Setter for m_cfaOffset member.
     * Set m_cfaOffset with given cfaOffset
     *
     * @param  cfaOffset Offset to be assigned to m_cfaOffset
     *
     * @pre    true
     * @post   CfaOffset() = cfaOffset
     *
     */
    void
        CfaOffset(INT cfaOffset)
    {
        m_cfaOffset = cfaOffset;
    }

    /**
     * Setter for m_opOffset member.
     * Set m_opOffset with given opOffset
     *
     * @param  opOffset Offset to be assigned to m_cfaOffset
     *
     * @pre    true
     * @post   OpOffset() = opOffset
     *
     */    
    void
        OpOffset(INT opOffset)
    {
        m_opOffset = opOffset;
    }

    /**
     * Setter for m_section member.
     * Set m_section with given section
     *
     * @param  section Elf section to be assigned to m_section
     *
     * @pre    true
     * @post   Section() = section
     *
     */    
    void
        Section(const ElfSection& section)
    {
        m_section = section;
    }

    /**
     * Operation offset.
     */
    INT m_opOffset;

    /**
     * CFA offset.
     */
    INT m_cfaOffset;

    /**
     * Elf section
     */
    ElfSection m_section;
};

#endif // ifndef __TARG_CGDWARF_UTILS_H__
