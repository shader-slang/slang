// slang-ir.h
#pragma once

// This file defines the intermediate representation (IR) used for Slang
// shader code. This is a typed static single assignment (SSA) IR,
// similar in spirit to LLVM (but much simpler).
//

#include "../compiler-core/slang-source-loc.h"
#include "../compiler-core/slang-source-map.h"
#include "../core/slang-basic.h"
#include "../core/slang-memory-arena.h"
#include "slang-ast-type.h"
#include "slang-container-pool.h"
#include "slang-ir-insts-enum.h"
#include "slang-type-system-shared.h"

#include <functional>

//
#include "slang-ir.h.fiddle"

FIDDLE()
namespace Slang
{

class Decl;
class DiagnosticSink;
class GenericDecl;
class FuncType;
class Layout;
class Type;
class Session;
class Name;
struct IRBuilder;
struct IRFunc;
struct IRGlobalValueWithCode;
struct IRInst;
struct IRModule;
struct IRStructField;
struct IRStructKey;

FIDDLE(instStructForwardDecls())

typedef unsigned int IROpFlags;
enum : IROpFlags
{
    kIROpFlags_None = 0,
    kIROpFlag_Parent = 1 << 0,   ///< This op is a parent op
    kIROpFlag_UseOther = 1 << 1, ///< If set this op can use 'other bits' to store information
    kIROpFlag_Hoistable =
        1 << 2, ///< If set this op is a hoistable inst that needs to be deduplicated.
    kIROpFlag_Global =
        1 << 3, ///< If set this op should always be hoisted but should never be deduplicated.
};

/* IROpMeta describes values for the layout of IROps */
enum IROpMeta
{
    kIROpMeta_OtherShift =
        10, ///< Number of bits for op (shift right by this to get the other bits)
};

/* IROpMask contains bitmasks for accessing aspects of IROps */
enum IROpMask : std::underlying_type_t<IROp>
{
    kIROpMask_OpMask = 0x3ff, ///< Mask for just opcode
};

enum IRMemoryOrder
{
    kIRMemoryOrder_Relaxed = 0,
    kIRMemoryOrder_Acquire = 1,
    kIRMemoryOrder_Release = 2,
    kIRMemoryOrder_AcquireRelease = 3,
    kIRMemoryOrder_SeqCst = 4,
};

inline int32_t operator&(const IROpMask m, const IROp o)
{
    return int32_t{m} & int32_t{o};
}

inline int32_t operator&(const IROp o, const IROpMask m)
{
    return m & o;
}

IROp findIROp(const UnownedStringSlice& name);

// A logical operation/opcode in the IR
struct IROpInfo
{
    // What is the name/mnemonic for this operation
    char const* name;

    // How many required arguments are there
    // (not including the mandatory type argument)
    unsigned int fixedArgCount;

    // Flags to control how we emit additional info
    IROpFlags flags;

    bool isHoistable() const { return (flags & kIROpFlag_Hoistable) != 0; }
    bool isGlobal() const { return (flags & kIROpFlag_Global) != 0; }
};

// Look up the info for an op
IROpInfo getIROpInfo(IROp op);

// A use of another value/inst within an IR operation
struct IRUse
{
    IRInst* get() const { return usedValue; }
    IRInst* getUser() const { return user; }

    void init(IRInst* user, IRInst* usedValue);
    void set(IRInst* usedValue);
    void clear();

    // The instruction that is being used
    IRInst* usedValue = nullptr;

    // The instruction that is doing the using.
    IRInst* user = nullptr;

    // The next use of the same value
    IRUse* nextUse = nullptr;

    // A "link" back to where this use is referenced,
    // so that we can simplify updates.
    IRUse** prevLink = nullptr;

    void debugValidate();
};

struct IRBlock;
struct IRDecoration;
struct IRRate;
struct IRType;
struct IRAttr;

// A double-linked list of instruction
struct IRInstListBase
{
    IRInstListBase() {}

    IRInstListBase(IRInst* first, IRInst* last)
        : first(first), last(last)
    {
    }


    IRInst* first = nullptr;
    IRInst* last = nullptr;

    IRInst* getFirst() { return first; }
    IRInst* getLast() { return last; }

    struct Iterator
    {
        IRInst* inst;

        Iterator()
            : inst(nullptr)
        {
        }
        Iterator(IRInst* inst)
            : inst(inst)
        {
        }

        void operator++();
        IRInst* operator*() { return inst; }

        bool operator!=(Iterator const& i) { return inst != i.inst; }
    };

    Iterator begin();
    Iterator end();
};

// Specialization of `IRInstListBase` for the case where
// we know (or at least expect) all of the instructions
// to be of type `T`
template<typename T>
struct IRInstList : IRInstListBase
{
    IRInstList() {}

    IRInstList(T* first, T* last)
        : IRInstListBase(first, last)
    {
    }

    explicit IRInstList(IRInstListBase const& list)
        : IRInstListBase(list)
    {
    }

    T* getFirst() { return (T*)first; }
    T* getLast() { return (T*)last; }

    struct Iterator : public IRInstListBase::Iterator
    {
        Iterator() {}
        Iterator(IRInst* inst)
            : IRInstListBase::Iterator(inst)
        {
        }

        T* operator*() { return (T*)inst; }
    };

    Iterator begin() { return Iterator(first); }
    Iterator end();
};

template<typename T>
struct IRModifiableInstList
{
    IRInst* parent;
    List<IRInst*> workList;

    IRModifiableInstList() {}

    IRModifiableInstList(T* parent, T* first, T* last);

    T* getFirst() { return workList.getCount() ? (T*)workList.getFirst() : nullptr; }
    T* getLast() { return workList.getCount() ? (T*)workList.getLast() : nullptr; }

    struct Iterator
    {
        IRModifiableInstList<T>* list;
        Index position = 0;

        Iterator() {}
        Iterator(IRModifiableInstList<T>* inList, Index inPos)
            : list(inList), position(inPos)
        {
        }

        T* operator*() { return (T*)(list->workList[position]); }
        void operator++();

        bool operator!=(Iterator const& i) { return i.list != list || i.position != position; }
    };

    Iterator begin() { return Iterator(this, 0); }
    Iterator end() { return Iterator(this, workList.getCount()); }
};

template<typename T>
struct IRFilteredInstList : IRInstListBase
{
    IRFilteredInstList() {}

    IRFilteredInstList(IRInst* fst, IRInst* lst);

    explicit IRFilteredInstList(IRInstListBase const& list)
        : IRFilteredInstList(list.first, list.last)
    {
    }

    T* getFirst() { return (T*)first; }
    T* getLast() { return (T*)last; }

    struct Iterator : public IRInstListBase::Iterator
    {
        IRInst* exclusiveLast;
        Iterator() {}
        Iterator(IRInst* inst, IRInst* lastIter)
            : IRInstListBase::Iterator(inst), exclusiveLast(lastIter)
        {
        }
        void operator++();
        T* operator*() { return (T*)inst; }
    };

    Iterator begin();
    Iterator end();
};

/// A list of contiguous operands that can be iterated over as `IRInst`s.
struct IROperandListBase
{
    IROperandListBase()
        : m_begin(nullptr), m_end(nullptr)
    {
    }

    IROperandListBase(IRUse* begin, IRUse* end)
        : m_begin(begin), m_end(end)
    {
    }

    struct Iterator
    {
    public:
        Iterator()
            : m_cursor(nullptr)
        {
        }

        Iterator(IRUse* use)
            : m_cursor(use)
        {
        }

        IRInst* operator*() const { return m_cursor->get(); }

        IRUse* getCursor() const { return m_cursor; }

        void operator++() { m_cursor++; }

        bool operator!=(Iterator const& that) const { return m_cursor != that.m_cursor; }

    protected:
        IRUse* m_cursor;
    };

    Iterator begin() const { return Iterator(m_begin); }
    Iterator end() const { return Iterator(m_end); }

    Int getCount() const { return m_end - m_begin; }

    IRInst* operator[](Int index) const { return m_begin[index].get(); }

protected:
    IRUse* m_begin;
    IRUse* m_end;
};

/// A list of contiguous operands that can be iterated over as all being of type `T`.
template<typename T>
struct IROperandList : IROperandListBase
{
    typedef IROperandListBase Super;

public:
    IROperandList() {}

    IROperandList(IRUse* begin, IRUse* end)
        : Super(begin, end)
    {
    }

    struct Iterator : public IROperandListBase::Iterator
    {
        typedef IROperandListBase::Iterator Super;

    public:
        Iterator() {}

        Iterator(IRUse* use)
            : Super(use)
        {
        }

        T* operator*() const { return (T*)m_cursor->get(); }
    };

    Iterator begin() const { return Iterator(m_begin); }
    Iterator end() const { return Iterator(m_end); }

    T* operator[](Int index) const { return (T*)m_begin[index].get(); }
};

/// A marker for a place where IR instructions can be inserted
///
/// An insertion location is defined relative to an existing IR
/// instruction, along with an enumeration that specifies where
/// new instructions should be inserted relative to the existing one.
///
/// Available options are:
///
/// * `None`, meaning the instruction is null/absent. This can either
///   represent an invalid/unitialized location, or an intention for
///   new instructions to be created without any parent.
///
/// * `AtEnd`, meaning new instructions will be inserted as the last
///   child of the existing instruction. This is useful for filling
///   in the children of a basic block or other container for a sequence
///   of instructions. Note that since each new instruction will become
///   the last one in the parent, instructions emitted at such a location
///   will appear in the same order that they were emitted.
///
/// * `Before`, meaning new instructions will be inserted before the existing
///   one. This is useful for inserting new instructions to compute a value
///   needed during optimization of an existing instruction (including when
///   the new instructions will *replace* the existing one). Because each
///   new instruction is inserted right before the existing one, the instructions
///   will appear in the same order that they were emitted.
///
/// * `AtStart`, meaning new instructions will be inserted as the first
///   child of the existing instruction. This is useful for adding things
///   like decorations to an existing instruction (since decorations are
///   currently required to precede all other kinds of child instructions).
///   Note that if multiple new instructions are inserted in this mode they
///   will appear in the *reverse* of the order they were emitted.
///
/// * `After`, meaning new instructions will be inserted as the next child
///   after the existing instruction.
///   Note that if multiple new instructions are inserted in this mode they
///   will appear in the *reverse* of the order they were emitted.
///
/// An insertion location is usable and valid so long as the instruction it is
/// defined relative to is valid to insert into or next to. If the reference
/// instruction is moved, subsequent insertions will use its new location, but
/// already-inserted instructions will *not*.
///
/// Note that at present there is no way to construct an `IRInsertLoc` that
/// can reliably be used to insert at certain locations that can be clearly
/// defined verbally (e.g., "at the end of the parameter list of this function").
/// Often a suitable approximation will work inside a specific pass (e.g., when
/// first constructing a function, the `AtEnd` mode could be used to insert
/// all parameters before any body instructions are inserted, and for an existing
/// function new parameters could be inserted `Before` the first existing body
/// instruction). Such approximations require knowing which kinds of IR modifications
/// will and will not be performed while the location is in use.
///
struct IRInsertLoc
{
public:
    /// The different kinds of insertion locations.
    enum class Mode
    {
        None,    //< Don't insert new instructions at all; just create them
        Before,  //< Insert immediately before the existing instruction
        After,   //< Insert immediately after the existing instruction
        AtStart, //< Insert at the start of the existing instruction's child list
        AtEnd,   //< Insert at the start of the existing instruction's child list
    };

    /// Construct a default insertion location in the `None` mode.
    IRInsertLoc() {}

    /// Construct a location that inserts before `inst`
    static IRInsertLoc before(IRInst* inst)
    {
        SLANG_ASSERT(inst);
        return IRInsertLoc(Mode::Before, inst);
    }

    /// Construct a location that inserts after `inst`
    ///
    /// Note: instructions inserted at this location will appear in the opposite
    /// of the order they were emitted.
    static IRInsertLoc after(IRInst* inst)
    {
        SLANG_ASSERT(inst);
        return IRInsertLoc(Mode::After, inst);
    }

    /// Construct a location that inserts at the start of the child list for `parent`
    ///
    /// Note: instructions inserted at this location will appear in the opposite
    /// of the order they were emitted.
    static IRInsertLoc atStart(IRInst* parent)
    {
        SLANG_ASSERT(parent);
        return IRInsertLoc(Mode::AtStart, parent);
    }

    /// Construct a location that inserts at the end of the child list for `parent`
    static IRInsertLoc atEnd(IRInst* parent)
    {
        SLANG_ASSERT(parent);
        return IRInsertLoc(Mode::AtEnd, parent);
    }

    /// Get the insertion mode for this location
    Mode getMode() const { return m_mode; }

    /// Get the instruction that this location inserts relative to
    IRInst* getInst() const { return m_inst; }

    /// Get the parent instruction that new instructions will insert into.
    ///
    /// For the `AtStart` and `AtEnd` modes, this returns `getInst()`.
    /// For the `Before` and `After` modes, this returns `getInst()->getParent()`
    IRInst* getParent() const;

    /// Get the parent basic block, if any, that new instructions will insert into.
    ///
    /// This returns the same instruction as `getParent()` if the parent is a basic block.
    /// Otherwise, returns null.
    IRBlock* getBlock() const;

    /// Get the enclosing function (or other code-bearing value) that instructions are inserted
    /// into.
    ///
    /// This searches up the parent chain starting with `getParent()` looking for a code-bearing
    /// value that things are being inserted into (could be a function, generic, etc.)
    ///
    IRInst* getFunc() const;

private:
    /// Internal constructor
    IRInsertLoc(Mode mode, IRInst* inst)
        : m_mode(mode), m_inst(inst)
    {
    }

    /// The insertion mode
    Mode m_mode = Mode::None;

    /// The instruction that insertions will be made relative to.
    ///
    /// Should always be null for the `None` mode and non-null for all other modes.
    IRInst* m_inst = nullptr;
};

enum class SideEffectAnalysisOptions
{
    None,
    UseDominanceTree,
};

enum class IRTypeLayoutRuleName
{
    Natural,
    Scalar = Natural,
    Std430,
    Std140,
    D3DConstantBuffer,
    _Count,
};

struct IRBlock;

// Every value in the IR is an instruction (even things
// like literal values).
//
FIDDLE()
struct IRInst
{
    FIDDLE(...)
    // The operation that this value represents
    IROp m_op;

    IROp getOp() const { return m_op; }

    // The total number of operands of this instruction.
    //
    // TODO: We shouldn't need to allocate this on
    // all instructions. Instead we should have
    // instructions that need "vararg" support to
    // allocate this field ahead of the `this`
    // pointer.
    uint32_t operandCount = 0;

    UInt getOperandCount() { return operandCount; }

    // Source location information for this value, if any
    SourceLoc sourceLoc;

    // Each instruction can have zero or more "decorations"
    // attached to it. A decoration is a specialized kind
    // of instruction that either attaches metadata to,
    // or modifies the semantics of, its parent instruction.
    //
    IRDecoration* getFirstDecoration();
    IRDecoration* getLastDecoration();
    IRInstList<IRDecoration> getDecorations();

    // Look up a decoration in the list of decorations
    IRDecoration* findDecorationImpl(IROp op);
    template<typename T>
    T* findDecoration();

    /// Get all the attributes attached to this instruction.
    IROperandList<IRAttr> getAllAttrs();

    /// Find the first attribute of type `T` attached to this instruction.
    template<typename T>
    T* findAttr()
    {
        for (auto attr : getAllAttrs())
        {
            if (auto found = as<T>(attr))
                return found;
        }
        return nullptr;
    }

    /// Find all attributes of type `T` attached to this instruction.
    ///
    /// This operation assumes that attributes are grouped by type,
    /// so that all the attributes of type `T` are contiguous.
    ///
    template<typename T>
    IROperandList<T> findAttrs()
    {
        auto allAttrs = getAllAttrs();
        auto bb = allAttrs.begin();
        auto end = allAttrs.end();
        while (bb != end && !as<T>(*bb))
            ++bb;
        auto ee = bb;
        while (ee != end && as<T>(*ee))
            ++ee;
        return IROperandList<T>(bb.getCursor(), ee.getCursor());
    }

    // The first use of this value (start of a linked list)
    IRUse* firstUse = nullptr;


    // The parent of this instruction.
    IRInst* parent;

    IRInst* getParent() { return parent; }

    // The next and previous instructions with the same parent
    IRInst* next;
    IRInst* prev;

    IRInst* getNextInst() { return next; }
    IRInst* getPrevInst() { return prev; }

    // An instruction can have zero or more children, although
    // only certain instruction opcodes are allowed to have
    // children.
    //
    // For example, a function will have children that are
    // its basic blocks, and the basic blocks will have children
    // that represent parameters and ordinary executable instructions.
    //
    IRInst* getFirstChild();
    IRInst* getLastChild();
    IRInstList<IRInst> getChildren() { return IRInstList<IRInst>(getFirstChild(), getLastChild()); }

    IRModifiableInstList<IRInst> getModifiableChildren()
    {
        return IRModifiableInstList<IRInst>(this, getFirstChild(), getLastChild());
    }

    /// A doubly-linked list containing any decorations and then any children of this instruction.
    ///
    /// We store both the decorations and children of an instruction
    /// in the same list, to conserve space in the instruction itself
    /// (rather than storing distinct lists for decorations and children).
    ///
    // Note: This field is *not* being declared `private` because doing so could
    // mess with our required memory layout, where `typeUse` below is assumed
    // to be the last field in `IRInst` and to come right before any additional
    // `IRUse` values that represent operands.
    //
    IRInstListBase m_decorationsAndChildren;


    IRInst* getFirstDecorationOrChild() { return m_decorationsAndChildren.first; }
    IRInst* getLastDecorationOrChild() { return m_decorationsAndChildren.last; }
    IRInstListBase getDecorationsAndChildren() { return m_decorationsAndChildren; }
    IRModifiableInstList<IRInst> getModifiableDecorationsAndChildren()
    {
        return IRModifiableInstList<IRInst>(
            this,
            m_decorationsAndChildren.first,
            m_decorationsAndChildren.last);
    }
    void removeAndDeallocateAllDecorationsAndChildren();
    bool hasDecorationOrChild() { return m_decorationsAndChildren.first != nullptr; }

#ifdef SLANG_ENABLE_IR_BREAK_ALLOC
    // Unique allocation ID for this instruction since start of current process.
    // Used to aid debugging only.
    uint32_t _debugUID;
#endif

    // Reserved memory space for use by individual IR passes.
    // This field is not supposed to be valid outside an IR pass,
    // and each IR pass should always treat it as uninitialized
    // upon entry.
    UInt64 scratchData = 0;

    // The type of the result value of this instruction,
    // or `null` to indicate that the instruction has
    // no value.
    IRUse typeUse;

    IRType* getFullType() { return (IRType*)typeUse.get(); }
    void setFullType(IRType* type) { typeUse.init(this, (IRInst*)type); }

    IRRate* getRate();

    IRType* getDataType();

    // After the type, we have data that is specific to
    // the subtype of `IRInst`. In most cases, this is
    // just a series of `IRUse` values representing
    // operands of the instruction.

    IRUse* getOperands();

    IRInst* getOperand(UInt index)
    {
        SLANG_ASSERT(index < getOperandCount());
        return getOperands()[index].get();
    }

    void setOperand(UInt index, IRInst* value)
    {
        SLANG_ASSERT(getOperands()[index].user != nullptr);
        getOperands()[index].set(value);
    }

    void unsafeSetOperand(UInt index, IRInst* value)
    {
        SLANG_ASSERT(getOperands()[index].user != nullptr);
        getOperands()[index].init(this, value);
    }

    //

    // Replace all uses of this value with `other`, so
    // that this value will now have no uses.
    void replaceUsesWith(IRInst* other);

    void insertAt(IRInsertLoc const& loc);

    // Insert this instruction into the same basic block
    // as `other`, right before/after it.
    void insertBefore(IRInst* other);
    void insertAfter(IRInst* other);

    // Insert as first/last child of given parent
    void insertAtStart(IRInst* parent);
    void insertAtEnd(IRInst* parent);

    // Move to the start/end of current parent
    void moveToStart();
    void moveToEnd();

    // Remove this instruction from its parent block,
    // but don't delete it, or replace uses.
    void removeFromParent();

    // Remove this instruction from its parent block,
    // and then destroy it (it had better have no uses!)
    void removeAndDeallocate();

    // Clear out the arguments of this instruction,
    // so that we don't appear on the list of uses
    // for those values.
    void removeArguments();

    // Remove operand `index` from operand list.
    // For example, if the inst is `op(a,b,c)`, calling removeOperand(inst, 1) will result
    // `op(a,c)`.
    void removeOperand(Index index);

    /// Transfer any decorations of this instruction to the `target` instruction.
    void transferDecorationsTo(IRInst* target);

    /// Does this instruction have any uses?
    bool hasUses() const { return firstUse != nullptr; }

    /// Does this instructiomn have more than one use?
    bool hasMoreThanOneUse() const { return firstUse != nullptr && firstUse->nextUse != nullptr; }

    /// It is possible that this instruction has side effects?
    ///
    /// This is a conservative test, and will return `true` if an exact answer can't be determined.
    bool mightHaveSideEffects(SideEffectAnalysisOptions options = SideEffectAnalysisOptions::None);

    // RTTI support
    static bool isaImpl(IROp) { return true; }

    /// Find the module that this instruction is nested under.
    ///
    /// If this instruction is transitively nested inside some IR module,
    /// this function will return it, and will otherwise return `null`.
    IRModule* getModule();

    /// Insert this instruction into `inParent`, after `inPrev` and before `inNext`.
    ///
    /// `inParent` must be non-null
    /// If `inPrev` is non-null it must satisfy `inPrev->getNextInst() == inNext` and
    /// `inPrev->getParent() == inParent` If `inNext` is non-null it must satisfy
    /// `inNext->getPrevInst() == inPrev` and `inNext->getParent() == inParent`
    ///
    /// If both `inPrev` and `inNext` are null, then `inParent` must have no (raw) children.
    ///
    void _insertAt(IRInst* inPrev, IRInst* inNext, IRInst* inParent);

    /// Print the IR to stdout for debugging purposes.
    ///
    void dump();

    /// Print the IR to a string for debugging purposes.
    ///
    void dump(String& outStr);

    /// Insert a basic block at the end of this func/code containing inst.
    void addBlock(IRBlock* block);

    IRBlock* getFirstBlock() { return (IRBlock*)getFirstChild(); }
    IRBlock* getLastBlock() { return (IRBlock*)getLastChild(); }
};

enum class IRDynamicCastBehavior
{
    Unwrap,
    NoUnwrap
};

template<typename T, IRDynamicCastBehavior behavior = IRDynamicCastBehavior::Unwrap>
T* dynamicCast(IRInst* inst)
{
    if (!inst)
        return nullptr;
    if (T::isaImpl(inst->getOp()))
        return static_cast<T*>(inst);
    if constexpr (behavior == IRDynamicCastBehavior::Unwrap)
    {
        if (inst->getOp() == kIROp_AttributedType)
            return dynamicCast<T>(inst->getOperand(0));
    }
    return nullptr;
}

template<typename T, IRDynamicCastBehavior behavior = IRDynamicCastBehavior::Unwrap>
const T* dynamicCast(const IRInst* inst)
{
    return dynamicCast<T, behavior>(const_cast<IRInst*>(inst));
}

// `dynamic_cast` equivalent (we just use dynamicCast)
template<typename T, IRDynamicCastBehavior behavior = IRDynamicCastBehavior::Unwrap>
T* as(IRInst* inst)
{
    return dynamicCast<T, behavior>(inst);
}

template<typename T, IRDynamicCastBehavior behavior = IRDynamicCastBehavior::Unwrap>
const T* as(const IRInst* inst)
{
    return dynamicCast<T, behavior>(inst);
}

// `static_cast` equivalent, with debug validation
template<typename T>
T* cast(IRInst* inst, T* /* */ = nullptr)
{
    SLANG_ASSERT(!inst || as<T>(inst));
    return (T*)inst;
}

SourceLoc const& getDiagnosticPos(IRInst* inst);

// Now that `IRInst` is defined we can back-fill the definitions that need to access it.

template<typename T>
T* IRInst::findDecoration()
{
    for (auto decoration : getDecorations())
    {
        if (auto match = as<T>(decoration))
            return match;
    }
    return nullptr;
}

template<typename T>
typename IRInstList<T>::Iterator IRInstList<T>::end()
{
    return Iterator(last ? last->next : nullptr);
}

template<typename T>
IRModifiableInstList<T>::IRModifiableInstList(T* inParent, T* first, T* last)
{
    parent = inParent;
    for (auto item = first; item; item = item->next)
    {
        workList.add(item);
        if (item == last)
            break;
    }
}

template<typename T>
void IRModifiableInstList<T>::Iterator::operator++()
{
    position++;
    while (position < list->workList.getCount())
    {
        auto inst = list->workList[position];
        if (!as<T>(inst))
        {
            // Skip insts that are not of type T.
        }
        else if (list->parent != inst->parent)
        {
            // Skip insts that are no longer in its original parent.
        }
        else
            break;
        position++;
    }
}

template<typename T>
IRFilteredInstList<T>::IRFilteredInstList(IRInst* fst, IRInst* lst)
{
    first = fst;
    last = lst;

    auto lastIter = last ? last->next : nullptr;
    while (first != lastIter && !as<T>(first))
        first = first->next;
    while (last && last != first && !as<T>(last))
        last = last->prev;
}

template<typename T>
void IRFilteredInstList<T>::Iterator::operator++()
{
    inst = inst->next;
    while (inst != exclusiveLast && !as<T>(inst))
    {
        inst = inst->next;
    }
}
template<typename T>
typename IRFilteredInstList<T>::Iterator IRFilteredInstList<T>::begin()
{
    auto lastIter = last ? last->next : nullptr;
    return IRFilteredInstList<T>::Iterator(first, lastIter);
}

template<typename T>
typename IRFilteredInstList<T>::Iterator IRFilteredInstList<T>::end()
{
    auto lastIter = last ? last->next : nullptr;
    return IRFilteredInstList<T>::Iterator(lastIter, lastIter);
}

// Types


// All types in the IR are represented as instructions which conceptually
// execute before run time.
FIDDLE()
struct IRType : IRInst
{
    FIDDLE(baseInst{noIsaImpl = true})
    IRType* getCanonicalType() { return this; }

    // Hack: specialize can also be a type. We should consider using a
    // separate `specializeType` op code for types so we can use the normal
    // isaImpl definition macro here.
    static bool isaImpl(IROp opIn)
    {
        const int op = (kIROpMask_OpMask & opIn);
        return (op >= kIROp_FirstType && op <= kIROp_LastType) || op == kIROp_Specialize;
    }
};

IRType* unwrapArray(IRType* type);

FIDDLE()
struct IRBasicType : IRType
{
    FIDDLE(baseInst())
    BaseType getBaseType() { return BaseType(getOp() - kIROp_FirstBasicType); }
};

FIDDLE()
struct IRVoidType : IRBasicType
{
    FIDDLE(leafInst())
};

FIDDLE()
struct IRBoolType : IRBasicType
{
    FIDDLE(leafInst())
};

FIDDLE()
struct IRStringTypeBase : IRType
{
    FIDDLE(baseInst())
};


// True if types are equal
// Note compares nominal types by name alone
bool isTypeEqual(IRType* a, IRType* b);

// True if this is an integral IRBasicType, not including Char or Ptr types
bool isIntegralType(IRType* t);

bool isFloatingType(IRType* t);

struct IntInfo
{
    Int width;
    bool isSigned;
    bool operator==(const IntInfo& i) const { return width == i.width && isSigned == i.isSigned; }
};

IntInfo getIntTypeInfo(const IRType* intType);

// left-inverse of getIntTypeInfo
IROp getIntTypeOpFromInfo(const IntInfo info);

IROp getOppositeSignIntTypeOp(IROp op);

struct FloatInfo
{
    Int width;
    bool operator==(const FloatInfo& i) const { return width == i.width; }
};

FloatInfo getFloatingTypeInfo(const IRType* floatType);

bool isIntegralScalarOrCompositeType(IRType* t);

IRStructField* findStructField(IRInst* type, IRStructKey* key);

void findAllInstsBreadthFirst(IRInst* inst, List<IRInst*>& outInsts);

// Constant Instructions

typedef int64_t IRIntegerValue;
typedef uint64_t IRUnsignedIntegerValue;
typedef double IRFloatingPointValue;

FIDDLE()
struct IRConstant : IRInst
{
    FIDDLE(baseInst())
    enum class FloatKind
    {
        Finite,
        PositiveInfinity,
        NegativeInfinity,
        Nan,
    };

    struct StringValue
    {
        uint32_t numChars; ///< The number of chars
        char chars[1];     ///< Chars added at end. NOTE! Must be last member of struct!
    };
    struct StringSliceValue
    {
        uint32_t numChars;
        char* chars;
    };

    union ValueUnion
    {
        IRIntegerValue intVal; ///< Used for integrals and boolean
        IRFloatingPointValue floatVal;
        void* ptrVal;

        /// Either of these types could be set with kIROp_StringLit.
        /// Which is used is currently determined with decorations - if a kIROp_TransitoryDecoration
        /// is set, then the transitory StringVal is used, else stringVal
        // which relies on chars being held after the struct).
        StringValue stringVal;
        StringSliceValue transitoryStringVal;
    };

    /// Returns a string slice (or empty string if not appropriate)
    UnownedStringSlice getStringSlice();

    /// Returns the kind of floating point value we have
    FloatKind getFloatKind() const;

    /// Returns true if the value is finite.
    /// NOTE! Only works on floating point types
    bool isFinite() const;

    /// True if constants are equal
    bool equal(IRConstant* rhs);
    /// True if the value is equal.
    /// Does *NOT* compare if the type is equal.
    bool isValueEqual(IRConstant* rhs);

    /// Get the hash
    HashCode getHashCode();


    // Must be last member, because data may be held behind
    // NOTE! The total size of IRConstant may not be allocated - only enough space is allocated for
    // the value type held in the union.
    ValueUnion value;
};

FIDDLE()
struct IRIntLit : IRConstant
{
    FIDDLE(leafInst())
    IRIntegerValue getValue() { return value.intVal; }
};

FIDDLE()
struct IRFloatLit : IRConstant
{
    FIDDLE(leafInst())
    IRFloatingPointValue getValue() { return value.floatVal; }
};

FIDDLE()
struct IRBoolLit : IRConstant
{
    FIDDLE(leafInst())
    bool getValue() { return value.intVal != 0; }
};

// Get the compile-time constant integer value of an instruction,
// if it has one, and assert-fail otherwise.
IRIntegerValue getIntVal(IRInst* inst);

// If it's a specialization constant sized array or unsized array, returns
// kUnsizedArrayMagicLength if it's an unsized array. Otherwise just returns
// the actual size.
IRIntegerValue getArraySizeVal(IRInst* inst);

FIDDLE()
struct IRStringLit : IRConstant
{
    FIDDLE(leafInst())
};

FIDDLE()
struct IRBlobLit : IRConstant
{
    FIDDLE(leafInst())
};

FIDDLE()
struct IRPtrLit : IRConstant
{
    FIDDLE(leafInst())

    void* getValue() { return value.ptrVal; }
};

FIDDLE()
struct IRVoidLit : IRConstant
{
    FIDDLE(leafInst())
};

// A instruction that ends a basic block (usually because of control flow)
FIDDLE()
struct IRTerminatorInst : IRInst
{
    FIDDLE(baseInst())
};

// A function parameter is owned by a basic block, and represents
// either an incoming function parameter (in the entry block), or
// a value that flows from one SSA block to another (in a non-entry
// block).
//
// In each case, the basic idea is that a block is a "label with
// arguments."
//
FIDDLE()
struct IRParam : IRInst
{
    FIDDLE(leafInst())
    IRParam* getNextParam();
    IRParam* getPrevParam();
};

/// A control-flow edge from one basic block to another
struct IREdge
{
public:
    IREdge() {}

    explicit IREdge(IRUse* use)
        : m_use(use)
    {
    }

    IRBlock* getPredecessor() const;
    IRBlock* getSuccessor() const;

    IRUse* getUse() const { return m_use; }

    bool isCritical() const;

private:
    IRUse* m_use = nullptr;
};

// A basic block is a parent instruction that adds the constraint
// that all the children need to be "ordinary" instructions (so
// no function declarations, or nested blocks). We also expect
// that the previous/next instruction are always a basic block.
//
FIDDLE()
struct IRBlock : IRInst
{
    FIDDLE(leafInst())
    // Linked list of the instructions contained in this block
    //
    IRInst* getFirstInst() { return getChildren().first; }
    IRInst* getLastInst() { return getChildren().last; }

    // In a valid program, every basic block should end with
    // a "terminator" instruction.
    //
    // This function will return the terminator, if it exists,
    // or `null` if there is none.
    IRTerminatorInst* getTerminator() { return as<IRTerminatorInst>(getLastDecorationOrChild()); }

    // We expect that the siblings of a basic block will
    // always be other basic blocks (we don't allow
    // mixing of blocks and other instructions in the
    // same parent).
    //
    // The exception to this is that decorations on the function
    // that contains a block could appear before the first block,
    // so we need to be careful to do a dynamic cast (`as`) in
    // the `getPrevBlock` case, but don't need to worry about
    // it for `getNextBlock`.
    IRBlock* getPrevBlock() { return as<IRBlock>(getPrevInst()); }
    IRBlock* getNextBlock() { return cast<IRBlock>(getNextInst()); }

    // The parameters of a block are represented by `IRParam`
    // instructions at the start of the block. These play
    // the role of function parameters for the entry block
    // of a function, and of phi nodes in other blocks.
    IRParam* getFirstParam()
    {
        return as<IRParam, IRDynamicCastBehavior::NoUnwrap>(getFirstInst());
    }
    IRParam* getLastParam();
    IRInstList<IRParam> getParams() { return IRInstList<IRParam>(getFirstParam(), getLastParam()); }
    // Linear in the parameter index, returns -1 if the param doesn't exist
    Index getParamIndex(IRParam* const needle)
    {
        Index ret = 0;
        for (const auto p : getParams())
        {
            if (p == needle)
                return ret;
            ret++;
        }
        return -1;
    }

    void addParam(IRParam* param);
    void insertParamAtHead(IRParam* param);

    // The "ordinary" instructions come after the parameters
    IRInst* getFirstOrdinaryInst();
    IRInst* getLastOrdinaryInst();
    IRInstList<IRInst> getOrdinaryInsts()
    {
        return IRInstList<IRInst>(getFirstOrdinaryInst(), getLastOrdinaryInst());
    }

    // The predecessor and successor lists of a block are needed
    // when we want to work with the control flow graph (CFG) of
    // a function. Rather than store these explicitly (and thus
    // need to update them when transformations might change the
    // CFG), we compute predecessors and successors in an
    // implicit fashion using the use-def information for a
    // block itself.
    //
    // To a first approximation, the predecessors of a block
    // are the blocks where the IR value of the block is used.
    // Similarly, the successors of a block are all values used
    // by the terminator instruction of the block.
    // The `getPredecessors()` and `getSuccessors()` functions
    // make this more precise.
    //
    struct PredecessorList
    {
        PredecessorList(IRUse* begin)
            : b(begin)
        {
        }
        IRUse* b;

        UInt getCount();
        bool isEmpty();

        struct Iterator
        {
            Iterator(IRUse* use)
                : use(use)
            {
            }

            IRBlock* operator*();

            void operator++();

            bool operator!=(Iterator const& that) { return use != that.use; }

            IREdge getEdge() const { return IREdge(use); }
            IRUse* use;
        };

        Iterator begin() { return Iterator(b); }
        Iterator end() { return Iterator(nullptr); }
    };

    struct SuccessorList
    {
        SuccessorList(IRUse* begin, IRUse* end, UInt stride = 1)
            : begin_(begin), end_(end), stride(stride)
        {
        }
        IRUse* begin_;
        IRUse* end_;
        UInt stride;

        UInt getCount();

        struct Iterator
        {
            Iterator(IRUse* use, UInt stride)
                : use(use), stride(stride)
            {
            }

            IRBlock* operator*();

            void operator++();

            bool operator!=(Iterator const& that) { return use != that.use; }

            IREdge getEdge() const { return IREdge(use); }

            IRUse* use;
            UInt stride;
        };

        Iterator begin() { return Iterator(begin_, stride); }
        Iterator end() { return Iterator(end_, stride); }
    };

    PredecessorList getPredecessors();
    SuccessorList getSuccessors();

    //
};


FIDDLE()
struct IRResourceTypeBase : IRType
{
    FIDDLE(baseInst())
    IRInst* getShapeInst() { return getOperand(kCoreModule_TextureShapeParameterIndex); }
    IRInst* getIsArrayInst() { return getOperand(kCoreModule_TextureIsArrayParameterIndex); }
    IRInst* getIsMultisampleInst()
    {
        return getOperand(kCoreModule_TextureIsMultisampleParameterIndex);
    }
    IRInst* getSampleCountInst()
    {
        return getOperand(kCoreModule_TextureSampleCountParameterIndex);
    }
    IRInst* getAccessInst() { return getOperand(kCoreModule_TextureAccessParameterIndex); }
    IRInst* getIsShadowInst() { return getOperand(kCoreModule_TextureIsShadowParameterIndex); }
    IRInst* getIsCombinedInst() { return getOperand(kCoreModule_TextureIsCombinedParameterIndex); }
    IRInst* getFormatInst() { return getOperand(kCoreModule_TextureFormatParameterIndex); }

    SlangResourceShape GetBaseShape()
    {
        switch (getOperand(1)->getOp())
        {
        case kIROp_TextureShape1DType:
            return SLANG_TEXTURE_1D;
        case kIROp_TextureShape2DType:
            return SLANG_TEXTURE_2D;
        case kIROp_TextureShape3DType:
            return SLANG_TEXTURE_3D;
        case kIROp_TextureShapeCubeType:
            return SLANG_TEXTURE_CUBE;
        case kIROp_TextureShapeBufferType:
            return SLANG_TEXTURE_BUFFER;
        default:
            return SLANG_RESOURCE_NONE;
        }
    }
    bool isFeedback() { return getIntVal(getAccessInst()) == kCoreModule_ResourceAccessFeedback; }
    bool isMultisample() { return getIntVal(getIsMultisampleInst()) != 0; }
    bool isArray() { return getIntVal(getIsArrayInst()) != 0; }
    bool isShadow() { return getIntVal(getIsShadowInst()) != 0; }
    bool isCombined() { return getIntVal(getIsCombinedInst()) != 0; }

    SlangResourceShape getShape()
    {
        return (SlangResourceShape)((uint32_t)GetBaseShape() |
                                    (isArray() ? SLANG_TEXTURE_ARRAY_FLAG : SLANG_RESOURCE_NONE));
    }
    SlangResourceAccess getAccess()
    {
        auto constVal = as<IRIntLit>(getOperand(kCoreModule_TextureAccessParameterIndex));
        if (constVal)
        {
            switch (getIntVal(constVal))
            {
            case kCoreModule_ResourceAccessReadOnly:
                return SLANG_RESOURCE_ACCESS_READ;
            case kCoreModule_ResourceAccessReadWrite:
                return SLANG_RESOURCE_ACCESS_READ_WRITE;
            case kCoreModule_ResourceAccessRasterizerOrdered:
                return SLANG_RESOURCE_ACCESS_RASTER_ORDERED;
            case kCoreModule_ResourceAccessFeedback:
                return SLANG_RESOURCE_ACCESS_FEEDBACK;
            case kCoreModule_ResourceAccessWriteOnly:
                return SLANG_RESOURCE_ACCESS_WRITE;
            default:
                break;
            }
        }
        return SLANG_RESOURCE_ACCESS_UNKNOWN;
    }
};

FIDDLE()
struct IRResourceType : IRResourceTypeBase
{
    FIDDLE(baseInst())
    IRType* getElementType() { return (IRType*)getOperand(0); }
    IRInst* getSampleCount() { return getSampleCountInst(); }
    bool hasFormat() { return getOperandCount() >= 9; }
    IRIntegerValue getFormat() { return getIntVal(getFormatInst()); }
};

FIDDLE()
struct IRTextureTypeBase : IRResourceType
{
    FIDDLE(baseInst())
};

FIDDLE()
struct IRTextureType : IRTextureTypeBase
{
    FIDDLE(leafInst())
};

FIDDLE()
struct IRGLSLImageType : IRTextureTypeBase
{
    FIDDLE(leafInst())
};

FIDDLE()
struct IRSubpassInputType : IRType
{
    FIDDLE(leafInst())
    IRType* getElementType() { return (IRType*)getOperand(0); }
    IRInst* getIsMultisampleInst() { return getOperand(1); }
    bool isMultisample() { return getIntVal(getIsMultisampleInst()) == 1; }
};

FIDDLE()
struct IRSamplerStateTypeBase : IRType
{
    FIDDLE(baseInst())
};


FIDDLE()
struct IRBuiltinGenericType : IRType
{
    FIDDLE(baseInst())
    IRType* getElementType() { return (IRType*)getOperand(0); }
};


FIDDLE()
struct IRHLSLStructuredBufferTypeBase : IRBuiltinGenericType
{
    FIDDLE(baseInst())
    IRType* getDataLayout() { return (IRType*)getOperand(1); }
};


FIDDLE()
struct IRHLSLPatchType : IRType
{
    FIDDLE(baseInst())
    IRType* getElementType() { return (IRType*)getOperand(0); }
    IRInst* getElementCount() { return getOperand(1); }
};


// Mesh shaders
// TODO: Ellie, should this parent struct be shared with Patch?
// IRArrayLikeType? IROpaqueArrayLikeType?
FIDDLE()
struct IRMeshOutputType : IRType
{
    FIDDLE(baseInst())
    IRType* getElementType() { return (IRType*)getOperand(0); }
    IRInst* getMaxElementCount() { return getOperand(1); }
};


FIDDLE()
struct IRMetalMeshType : IRType
{
    FIDDLE(leafInst())

    IRType* getVerticesType() { return (IRType*)getOperand(0); }
    IRType* getPrimitivesType() { return (IRType*)getOperand(1); }
    IRInst* getNumVertices() { return (IRInst*)getOperand(2); }
    IRInst* getNumPrimitives() { return (IRInst*)getOperand(3); }
    IRIntLit* getTopology() { return (IRIntLit*)getOperand(4); }
};

FIDDLE()
struct IRPointerLikeType : IRBuiltinGenericType
{
    FIDDLE(baseInst())
};

FIDDLE()
struct IRParameterGroupType : IRPointerLikeType
{
    FIDDLE(baseInst())
};

FIDDLE()
struct IRUniformParameterGroupType : IRParameterGroupType
{
    FIDDLE(baseInst())

    IRType* getDataLayout() { return getOperandCount() > 1 ? (IRType*)getOperand(1) : nullptr; }
};


FIDDLE()
struct IRGLSLShaderStorageBufferType : IRBuiltinGenericType
{
    FIDDLE(leafInst())
    IRType* getDataLayout() { return (IRType*)getOperand(1); }
};

FIDDLE()
struct IRArrayTypeBase : IRType
{
    FIDDLE(baseInst())
    IRType* getElementType() { return (IRType*)getOperand(0); }

    // Returns the element count for an `IRArrayType`, and null
    // for an `IRUnsizedArrayType`.
    IRInst* getElementCount();

    IRInst* getArrayStride()
    {
        switch (m_op)
        {
        case kIROp_ArrayType:
            if (getOperandCount() == 3)
                return getOperand(2);
            return nullptr;

        case kIROp_UnsizedArrayType:
            if (getOperandCount() == 2)
                return getOperand(1);
            return nullptr;
        }
        return nullptr;
    }
};

FIDDLE()
struct IRArrayType : IRArrayTypeBase
{
    FIDDLE(leafInst())
};


FIDDLE()
struct IRAtomicType : IRType
{
    FIDDLE(leafInst())

    IRType* getElementType() { return (IRType*)getOperand(0); }
};


FIDDLE()
struct IRRateQualifiedType : IRType
{
    FIDDLE(leafInst())
    IRRate* getRate() { return (IRRate*)getOperand(0); }
    IRType* getValueType() { return (IRType*)getOperand(1); }
};

FIDDLE()
struct IRDescriptorHandleType : IRType
{
    FIDDLE(leafInst())
    IRType* getResourceType() { return (IRType*)getOperand(0); }
};

// Unlike the AST-level type system where `TypeType` tracks the
// underlying type, the "type of types" in the IR is a simple
// value with no operands, so that all type nodes have the
// same type.

// The kind of any and all generics.
//
// A more complete type system would include "arrow kinds" to
// be able to track the domain and range of generics (e.g.,
// the `vector` generic maps a type and an integer to a type).
// This is only really needed if we ever wanted to support
// "higher-kinded" generics (e.g., a generic that takes another
// generic as a parameter).
//

FIDDLE()
struct IRDifferentialPairTypeBase : IRType
{
    FIDDLE(baseInst())
    IRType* getValueType() { return (IRType*)getOperand(0); }
    IRInst* getWitness() { return (IRInst*)getOperand(1); }
};

FIDDLE()
struct IRDifferentialPairType : IRDifferentialPairTypeBase
{
    FIDDLE(leafInst())
};

FIDDLE()
struct IRDifferentialPtrPairType : IRDifferentialPairTypeBase
{
    FIDDLE(leafInst())
};

FIDDLE()
struct IRDifferentialPairUserCodeType : IRDifferentialPairTypeBase
{
    FIDDLE(leafInst())
};

FIDDLE()
struct IRBackwardDiffIntermediateContextType : IRType
{
    FIDDLE(leafInst())
    IRInst* getFunc() { return getOperand(0); }
};

FIDDLE()
struct IRVectorType : IRType
{
    FIDDLE(leafInst())
    IRType* getElementType() { return (IRType*)getOperand(0); }
    IRInst* getElementCount() { return getOperand(1); }
};

FIDDLE()
struct IRMatrixType : IRType
{
    FIDDLE(leafInst())
    IRType* getElementType() { return (IRType*)getOperand(0); }
    IRInst* getRowCount() { return getOperand(1); }
    IRInst* getColumnCount() { return getOperand(2); }
    IRInst* getLayout() { return getOperand(3); }
};

FIDDLE()
struct IRArrayListType : IRType
{
    FIDDLE(leafInst())
    IRType* getElementType() { return (IRType*)getOperand(0); }
};

FIDDLE()
struct IRTensorViewType : IRType
{
    FIDDLE(leafInst())
    IRType* getElementType() { return (IRType*)getOperand(0); }
};

FIDDLE()
struct IRTorchTensorType : IRType
{
    FIDDLE(leafInst())
};

FIDDLE()
struct IRSPIRVLiteralType : IRType
{
    FIDDLE(leafInst())

    IRType* getValueType() { return static_cast<IRType*>(getOperand(0)); }
};

FIDDLE()
struct IRPtrTypeBase : IRType
{
    FIDDLE(baseInst())
    IRType* getValueType() { return (IRType*)getOperand(0); }

    bool hasAddressSpace()
    {
        return getOperandCount() > 1 && getAddressSpace() != AddressSpace::Generic;
    }

    AddressSpace getAddressSpace()
    {
        return getOperandCount() > 1
                   ? (AddressSpace) static_cast<IRIntLit*>(getOperand(1))->getValue()
                   : AddressSpace::Generic;
    }
};

FIDDLE()
struct IRComPtrType : public IRType
{
    FIDDLE(leafInst())
    IRType* getValueType() { return (IRType*)getOperand(0); }
};

FIDDLE()
struct IRNativePtrType : public IRType
{
    FIDDLE(leafInst())
    IRType* getValueType() { return (IRType*)getOperand(0); }
};

FIDDLE()
struct IRPseudoPtrType : public IRPtrTypeBase
{
    FIDDLE(leafInst())
};

/// The base class of RawPointerType and RTTIPointerType.
FIDDLE()
struct IRRawPointerTypeBase : IRType
{
    FIDDLE(baseInst())
};

/// Represents a pointer to an object of unknown type.
FIDDLE()
struct IRRawPointerType : IRRawPointerTypeBase
{
    FIDDLE(leafInst())
};

/// Represents a pointer to an object whose type is determined at runtime,
/// with type information available through `rttiOperand`.
///
FIDDLE()
struct IRRTTIPointerType : IRRawPointerTypeBase
{
    FIDDLE(leafInst())
    IRInst* getRTTIOperand() { return getOperand(0); }
};

FIDDLE()
struct IRGlobalHashedStringLiterals : IRInst
{
    FIDDLE(leafInst())
};

FIDDLE()
struct IRGetStringHash : IRInst
{
    FIDDLE(leafInst())

    IRStringLit* getStringLit() { return as<IRStringLit>(getOperand(0)); }
};

/// Get the type pointed to be `ptrType`, or `nullptr` if it is not a pointer(-like) type.
///
/// The given IR `builder` will be used if new instructions need to be created.
IRType* tryGetPointedToType(IRBuilder* builder, IRType* type);

FIDDLE()
struct IRFuncType : IRType
{
    FIDDLE(leafInst())
    IRType* getResultType() { return (IRType*)getOperand(0); }
    UInt getParamCount() { return getOperandCount() - 1; }
    IRType* getParamType(UInt index) { return (IRType*)getOperand(1 + index); }
    IROperandList<IRType> getParamTypes()
    {
        return IROperandList<IRType>(getOperands() + 1, getOperands() + getOperandCount());
    }
};

FIDDLE()
struct IRRayQueryType : IRType
{
    FIDDLE(leafInst())
};

FIDDLE()
struct IRHitObjectType : IRType
{
    FIDDLE(leafInst())
};

FIDDLE()
struct IRCoopVectorType : IRType
{
    FIDDLE(leafInst())
    IRType* getElementType() { return (IRType*)getOperand(0); }
    IRInst* getElementCount() { return getOperand(1); }
};

FIDDLE()
struct IRCoopMatrixType : IRType
{
    FIDDLE(leafInst())
    IRType* getElementType() { return (IRType*)getOperand(0); }
    IRInst* getScope() { return getOperand(1); }
    IRInst* getRowCount() { return getOperand(2); }
    IRInst* getColumnCount() { return getOperand(3); }
    IRInst* getMatrixUse() { return getOperand(4); }
};

FIDDLE()
struct IRTensorAddressingTensorLayoutType : IRType
{
    FIDDLE(leafInst())
    IRInst* getDimension() { return getOperand(0); }
    IRInst* getClampMode() { return getOperand(1); }
};

FIDDLE()
struct IRTensorAddressingTensorViewType : IRType
{
    FIDDLE(leafInst())
    IRInst* getDimension() { return getOperand(0); }
    IRInst* getHasDimension() { return getOperand(1); }
    IRInst* getPermutation(int index) { return getOperand(2 + index); }
};

bool isDefinition(IRInst* inVal);

// A structure type is represented as a parent instruction,
// where the child instructions represent the fields of the
// struct.
//
// The space of fields that a given struct type supports
// are defined as its "keys", which are global values
// (that is, they have mangled names that can be used
// for linkage).
//
FIDDLE()
struct IRStructKey : IRInst
{
    FIDDLE(leafInst())
};
//
// The fields of the struct are then defined as mappings
// from those keys to the associated type (in the case of
// the struct type) or to values (when lookup up a field).
//
// A struct field thus has two operands: the key, and the
// type of the field.
//
FIDDLE()
struct IRStructField : IRInst
{
    FIDDLE(leafInst())
    IRStructKey* getKey() { return cast<IRStructKey>(getOperand(0)); }
    IRType* getFieldType()
    {
        // Note: We do not use `cast` here because there are
        // cases of types (which we would like to conveniently
        // refer to via an `IRType*`) which do not actually
        // inherit from `IRType` in the hierarchy.
        //
        return (IRType*)getOperand(1);
    }
    void setFieldType(IRType* type) { setOperand(1, type); }
};
//
// The struct type is then represented as a parent instruction
// that contains the various fields. Note that a struct does
// *not* contain the keys, because code needs to be able to
// reference the keys from scopes outside of the struct.
//
FIDDLE()
struct IRStructType : IRType
{
    FIDDLE(leafInst())
    IRFilteredInstList<IRStructField> getFields()
    {
        return IRFilteredInstList<IRStructField>(getChildren());
    }
};

FIDDLE()
struct IRClassType : IRType
{
    FIDDLE(leafInst())
    IRFilteredInstList<IRStructField> getFields()
    {
        return IRFilteredInstList<IRStructField>(getChildren());
    }
};

FIDDLE()
struct IRAssociatedType : IRType
{
    FIDDLE(leafInst())
};

FIDDLE()
struct IRThisType : IRType
{
    FIDDLE(leafInst())

    IRInst* getConstraintType() { return getOperand(0); }
};

FIDDLE()
struct IRThisTypeWitness : IRInst
{
    FIDDLE(leafInst())

    IRInst* getConstraintType() { return getOperand(0); }
};

FIDDLE()
struct IRInterfaceRequirementEntry : IRInst
{
    FIDDLE(leafInst())
    IRInst* getRequirementKey() { return getOperand(0); }
    IRInst* getRequirementVal() { return getOperand(1); }
    void setRequirementKey(IRInst* val) { setOperand(0, val); }
    void setRequirementVal(IRInst* val) { setOperand(1, val); }
};

FIDDLE()
struct IRInterfaceType : IRType
{
    FIDDLE(leafInst())

    UInt getRequirementCount() { return getOperandCount(); }
};

FIDDLE()
struct IRConjunctionType : IRType
{
    FIDDLE(leafInst())

    Int getCaseCount() { return getOperandCount(); }
    IRType* getCaseType(Int index) { return (IRType*)getOperand(index); }
};

FIDDLE()
struct IRAttributedType : IRType
{
    FIDDLE(leafInst())

    IRType* getBaseType() { return (IRType*)getOperand(0); }
    IRInst* getAttr() { return getOperand(1); }
};

FIDDLE()
struct IRTupleTypeBase : IRType
{
    FIDDLE(baseInst())
};

/// Represents a tuple. Tuples are created by `IRMakeTuple` and its elements
/// are accessed via `GetTupleElement(tupleValue, IRIntLit)`.
FIDDLE()
struct IRTupleType : IRTupleTypeBase
{
    FIDDLE(leafInst())
};

/// Represents a type pack. Type packs behave like tuples, but they have a
/// "flattening" semantics, so that MakeTypePack(MakeTypePack(T1,T2), T3) is
/// MakeTypePack(T1,T2,T3).
FIDDLE()
struct IRTypePack : IRTupleTypeBase
{
    FIDDLE(leafInst())
};

// A placeholder struct key for tuple type layouts that will be replaced with
// the actual struct key when the tuple type is materialized into a struct type.
FIDDLE()
struct IRIndexedFieldKey : IRInst
{
    FIDDLE(leafInst())
    IRInst* getBaseType() { return getOperand(0); }
    IRInst* getIndex() { return getOperand(1); }
};

/// Represents a tuple in target language. TargetTupleType will not be lowered to structs.
FIDDLE()
struct IRTargetTupleType : IRType
{
    FIDDLE(leafInst())
};

/// Represents a `expand T` type used in variadic generic decls in Slang. Expected to be substituted
/// by actual types during specialization.
FIDDLE()
struct IRExpandTypeOrVal : IRType
{
    FIDDLE(leafInst())
    IRType* getPatternType() { return (IRType*)(getOperand(0)); }
    UInt getCaptureCount() { return getOperandCount() - 1; }
    IRType* getCaptureType(UInt index) { return (IRType*)(getOperand(index + 1)); }
};

/// Represents an `Result<T,E>`, used by functions that throws error codes.
FIDDLE()
struct IRResultType : IRType
{
    FIDDLE(leafInst())

    IRType* getValueType() { return (IRType*)getOperand(0); }
    IRType* getErrorType() { return (IRType*)getOperand(1); }
};

/// Represents an `Optional<T>`.
FIDDLE()
struct IROptionalType : IRType
{
    FIDDLE(leafInst())

    IRType* getValueType() { return (IRType*)getOperand(0); }
};

/// Represents an enum type
FIDDLE()
struct IREnumType : IRType
{
    FIDDLE(leafInst())

    IRType* getTagType() { return (IRType*)getOperand(0); }
};

FIDDLE()
struct IRTypeType : IRType
{
    FIDDLE(leafInst())
};

/// Represents the IR type for an `IRRTTIObject`.
FIDDLE()
struct IRRTTIType : IRType
{
    FIDDLE(leafInst())
};

/// Represents a handle to an RTTI object.
/// This is lowered as an integer number identifying a type.
FIDDLE()
struct IRRTTIHandleType : IRType
{
    FIDDLE(leafInst())
};

FIDDLE()
struct IRAnyValueType : IRType
{
    FIDDLE(leafInst())
    IRInst* getSize() { return getOperand(0); }
};

FIDDLE()
struct IRWitnessTableTypeBase : IRType
{
    FIDDLE(baseInst())
    IRInst* getConformanceType() { return getOperand(0); }
};

FIDDLE()
struct IRWitnessTableType : IRWitnessTableTypeBase
{
    FIDDLE(leafInst())
};

FIDDLE()
struct IRWitnessTableIDType : IRWitnessTableTypeBase
{
    FIDDLE(leafInst())
};

FIDDLE()
struct IRBindExistentialsTypeBase : IRType
{
    FIDDLE(baseInst())

    IRType* getBaseType() { return (IRType*)getOperand(0); }
    UInt getExistentialArgCount() { return getOperandCount() - 1; }
    IRUse* getExistentialArgs() { return getOperands() + 1; }
    IRInst* getExistentialArg(UInt index) { return getExistentialArgs()[index].get(); }
};

FIDDLE()
struct IRBindExistentialsType : IRBindExistentialsTypeBase
{
    FIDDLE(leafInst())
};

FIDDLE()
struct IRBoundInterfaceType : IRBindExistentialsTypeBase
{
    FIDDLE(leafInst())

    IRType* getInterfaceType() { return getBaseType(); }
    IRType* getConcreteType() { return (IRType*)getExistentialArg(0); }
    IRInst* getWitnessTable() { return getExistentialArg(1); }
};


/// @brief A global value that potentially holds executable code.
///
FIDDLE()
struct IRGlobalValueWithCode : IRInst
{
    FIDDLE(baseInst())
    // The children of a value with code will be the basic
    // blocks of its definition.
    IRBlock* getFirstBlock() { return cast<IRBlock>(getFirstChild()); }
    IRBlock* getLastBlock() { return cast<IRBlock>(getLastChild()); }
    IRInstList<IRBlock> getBlocks() { return IRInstList<IRBlock>(getChildren()); }
};

// A value that has parameters so that it can conceptually be called.
FIDDLE()
struct IRGlobalValueWithParams : IRGlobalValueWithCode
{
    FIDDLE(baseInst())
    // Convenience accessor for the IR parameters,
    // which are actually the parameters of the first
    // block.
    IRParam* getFirstParam();
    IRParam* getLastParam();
    IRInstList<IRParam> getParams();
    IRInst* getFirstOrdinaryInst();
};

// A function is a parent to zero or more blocks of instructions.
//
// A function is itself a value, so that it can be a direct operand of
// an instruction (e.g., a call).
FIDDLE()
struct IRFunc : IRGlobalValueWithParams
{
    FIDDLE(leafInst())
    // The type of the IR-level function
    IRFuncType* getDataType() { return (IRFuncType*)IRInst::getDataType(); }

    // Convenience accessors for working with the
    // function's type.
    IRType* getResultType();
    UInt getParamCount();
    IRType* getParamType(UInt index);

    bool isDefinition() { return getFirstBlock() != nullptr; }
};

/// Adjust the type of an IR function based on its parameter list.
///
/// The function type formed will use the types of the actual
/// parameters in the body of `func`, as well as the given `resultType`.
///
void fixUpFuncType(IRFunc* func, IRType* resultType);

/// Adjust the type of an IR function based on its parameter list.
///
/// The function type formed will use the types of the actual
/// parameters in the body of `func`, as well as the result type
/// that is found on the current type of `func`.
///
void fixUpFuncType(IRFunc* func);

// A generic is akin to a function, but is conceptually executed
// before runtime, to specialize the code nested within.
//
// In practice, a generic always holds only a single block, and ends
// with a `return` instruction for the value that the generic yields.
FIDDLE()
struct IRGeneric : IRGlobalValueWithParams
{
    FIDDLE(leafInst())
};

// Find the value that is returned from a generic, so that
// a pass can glean information from it.
IRInst* findGenericReturnVal(IRGeneric* generic);
// Recursively find the inner most generic return value.
IRInst* findInnerMostGenericReturnVal(IRGeneric* generic);

// Returns the generic return val if `inst` is a generic, otherwise returns `inst`.
IRInst* getGenericReturnVal(IRInst* inst);

// Find the generic container, if any, that this inst is contained in
// Returns nullptr if there is no outer container.
IRInst* findOuterGeneric(IRInst* inst);
// Recursively find the outer most generic container.
IRInst* findOuterMostGeneric(IRInst* inst);

// Returns `inst` if it is not a generic, otherwise its outer generic.
IRInst* maybeFindOuterGeneric(IRInst* inst);

struct IRSpecialize;
IRGeneric* findSpecializedGeneric(IRSpecialize* specialize);
IRInst* findSpecializeReturnVal(IRSpecialize* specialize);

// Resolve an instruction that might reference a static definition
// to the most specific IR node possible, so that we can read
// decorations from it (e.g., if this is a `specialize` instruction,
// then try to chase down the generic being specialized, and what
// it seems to return).
//
IRInst* getResolvedInstForDecorations(IRInst* inst, bool resolveThroughDifferentiation = false);

// The IR module itself is represented as an instruction, which
// serves at the root of the tree of all instructions in the module.
FIDDLE()
struct IRModuleInst : IRInst
{
    FIDDLE(leafInst())
    // Pointer back to the non-instruction object that represents
    // the module, so that we can get back to it in algorithms
    // that need it.
    IRModule* module;

    IRInstListBase getGlobalInsts() { return getChildren(); }
};

struct IRModule;

// Description of an instruction to be used for global value numbering
struct IRInstKey
{
private:
    IRInst* inst = nullptr;
    HashCode hashCode = 0;
    HashCode _getHashCode();

public:
    IRInstKey() = default;
    IRInstKey(const IRInstKey& key) = default;
    IRInstKey(IRInst* i)
        : inst(i)
    {
        hashCode = _getHashCode();
    }
    IRInstKey& operator=(const IRInstKey&) = default;
    HashCode getHashCode() const { return hashCode; }
    IRInst* getInst() const { return inst; }

    bool operator==(IRInstKey const& right) const
    {
        if (hashCode != right.getHashCode())
            return false;
        if (getInst()->getOp() != right.getInst()->getOp())
            return false;
        if (getInst()->getFullType() != right.getInst()->getFullType())
            return false;
        if (getInst()->operandCount != right.getInst()->operandCount)
            return false;

        auto argCount = getInst()->operandCount;
        auto leftArgs = getInst()->getOperands();
        auto rightArgs = right.getInst()->getOperands();
        for (UInt aa = 0; aa < argCount; ++aa)
        {
            if (leftArgs[aa].get() != rightArgs[aa].get())
                return false;
        }

        return true;
    }
};

struct IRConstantKey
{
    IRConstant* inst;

    bool operator==(const IRConstantKey& rhs) const { return inst->equal(rhs.inst); }
    HashCode getHashCode() const { return inst->getHashCode(); }
};

// State owned by IRModule for global value deduplication.
// Not supposed to be used/instantiated outside IRModule.
struct IRDeduplicationContext
{
public:
    IRDeduplicationContext(IRModule* module) { init(module); }

    void init(IRModule* module);

    IRModule* getModule() { return m_module; }

    Session* getSession() { return m_session; }

    void removeHoistableInstFromGlobalNumberingMap(IRInst* inst);

    void tryHoistInst(IRInst* inst);

    typedef Dictionary<IRInstKey, IRInst*> GlobalValueNumberingMap;
    typedef Dictionary<IRConstantKey, IRConstant*> ConstantMap;

    GlobalValueNumberingMap& getGlobalValueNumberingMap() { return m_globalValueNumberingMap; }
    Dictionary<IRInst*, IRInst*>& getInstReplacementMap() { return m_instReplacementMap; }

    void _addGlobalNumberingEntry(IRInst* inst)
    {
        m_globalValueNumberingMap.add(IRInstKey{inst}, inst);
        m_instReplacementMap.remove(inst);
        tryHoistInst(inst);
    }
    void _removeGlobalNumberingEntry(IRInst* inst)
    {
        IRInst* value = nullptr;
        if (m_globalValueNumberingMap.tryGetValue(IRInstKey{inst}, value))
        {
            if (value == inst)
            {
                m_globalValueNumberingMap.remove(IRInstKey{inst});
            }
        }
    }

    ConstantMap& getConstantMap() { return m_constantMap; }

private:
    // The module that will own all of the IR
    IRModule* m_module;

    // The parent compilation session
    Session* m_session;

    GlobalValueNumberingMap m_globalValueNumberingMap;

    // Duplicate insts that are still alive and needs to be replaced in m_globalValueNumberMap
    // when used as an operand to create another inst.
    Dictionary<IRInst*, IRInst*> m_instReplacementMap;

    ConstantMap m_constantMap;
};

struct IRDominatorTree;

struct IRAnalysis
{
    RefPtr<RefObject> domTree;
    IRDominatorTree* getDominatorTree();
};

FIDDLE()
struct IRModule : RefObject
{
    FIDDLE(...)
public:
    enum
    {
        kMemoryArenaBlockSize = 16 * 1024, ///< Use 16k block size for memory arena
    };

    static RefPtr<IRModule> create(Session* session);

    SLANG_FORCE_INLINE Session* getSession() const { return m_session; }
    SLANG_FORCE_INLINE IRModuleInst* getModuleInst() const { return m_moduleInst; }
    SLANG_FORCE_INLINE MemoryArena& getMemoryArena() { return m_memoryArena; }

    SLANG_FORCE_INLINE IBoxValue<SourceMap>* getObfuscatedSourceMap() const
    {
        return m_obfuscatedSourceMap;
    }
    SLANG_FORCE_INLINE void setObfuscatedSourceMap(IBoxValue<SourceMap>* sourceMap)
    {
        m_obfuscatedSourceMap = sourceMap;
    }

    ArrayView<IRInst*> findSymbolByMangledName(const ImmutableHashedString& mangledName) const
    {
        if (auto list = m_mapMangledNameToGlobalInst.tryGetValue(mangledName))
            return list->getArrayView();
        return {};
    }

    void buildMangledNameToGlobalInstMap();

    IRDeduplicationContext* getDeduplicationContext() const { return &m_deduplicationContext; }

    IRDominatorTree* findDominatorTree(IRGlobalValueWithCode* func)
    {
        IRAnalysis* analysis = m_mapInstToAnalysis.tryGetValue(func);
        if (analysis)
            return analysis->getDominatorTree();
        return nullptr;
    }
    IRDominatorTree* findOrCreateDominatorTree(IRGlobalValueWithCode* func);
    void invalidateAnalysisForInst(IRGlobalValueWithCode* func)
    {
        m_mapInstToAnalysis.remove(func);
    }
    void invalidateAllAnalysis() { m_mapInstToAnalysis.clear(); }

    IRInstListBase getGlobalInsts() const { return getModuleInst()->getChildren(); }

    Name* getName() const { return m_name; }
    void setName(Name* name) { m_name = name; }

    /// Create an empty instruction with the `op` opcode and space for
    /// a number of operands given by `operandCount`.
    ///
    /// The memory allocation will be *at least* `minSizeInBytes`, so
    /// if `sizeof(T)` is passed in the reuslt is guaranteed to be big
    /// enough for a `T` instance. It is safe to leave `minSizeInBytes` as zero
    /// for instructions where the only additional space they require is
    /// for their operands (which is most of them).
    ///
    /// The returned instruction is "empty" in thes sense that the `IRUse`s
    /// for its type and operands are *not* initialized. The caller takes
    /// full responsibility for initializing those uses as needed.
    ///
    /// This function does not (and cannot) perform any kind of deduplication
    /// or simplification. Clients take responsibility for only using this
    /// operation when they genuinely want a fresh instruction to be allocated.
    ///
    /// Note: the `_` prefix indicates that this is a low-level operation that
    /// must cient code should not be invoking. When in doubt, plase try to
    /// operations in `IRBuilder` to emit an instruction whenever possible.
    ///
    IRInst* _allocateInst(IROp op, Int operandCount, size_t minSizeInBytes = 0);

    template<typename T>
    T* _allocateInst(IROp op, Int operandCount)
    {
        return (T*)_allocateInst(op, operandCount, sizeof(T));
    }

    ContainerPool& getContainerPool() { return m_containerPool; }

    //
    // The range of module versions this compiler supports
    //
    // This will need to be updated if for example an instruction is removed,
    // the max supported version should be incremented and the min supported
    // version set to above the last version an instance of that instruction
    // could be found
    //
    // Additionally this should be updated when new instructions are added,
    // however only k_maxSupportedModuleVersion needs to be incremented in that
    // case
    //
    // It represents the version of module regarding semantics and doesn't have
    // anything to do with serialization format
    //
    const static UInt k_minSupportedModuleVersion = 0;
    const static UInt k_maxSupportedModuleVersion = 0;

private:
    friend struct IRSerialReadContext;
    friend struct IRSerialWriteContext;
    friend struct Fossilized_IRModule;

    IRModule() = delete;

    /// Ctor
    IRModule(Session* session)
        : m_session(session), m_memoryArena(kMemoryArenaBlockSize), m_deduplicationContext(this)
    {
    }

    // The compilation session in use.
    Session* m_session = nullptr;

    /// The root IR instruction for the module.
    ///
    /// All other IR instructions that make up the state/contents of the module are
    /// descendents of this instruction. Thus if we follow the chain of parent
    /// instructions from an arbitrary IR instruction we expect to find the
    /// `IRModuleInst` for the module the instruction belongs to, if any.
    ///
    FIDDLE() IRModuleInst* m_moduleInst = nullptr;

    // The name of the module.
    FIDDLE() Name* m_name = nullptr;

    // The version of the module as it was loaded
    FIDDLE() UInt m_version = k_maxSupportedModuleVersion;

    /// The memory arena from which all IR instructions (and any associated state) in this module
    /// are allocated.
    MemoryArena m_memoryArena;

    /// A pool to allow reuse of common types of containers to reduce memory allocations
    /// and rehashing.
    ContainerPool m_containerPool;

    /// Shared contexts for constructing and deduplicating the IR.
    mutable IRDeduplicationContext m_deduplicationContext;

    /// Holds the obfuscated source map for this module if applicable
    ComPtr<IBoxValue<SourceMap>> m_obfuscatedSourceMap;

    Dictionary<IRInst*, IRAnalysis> m_mapInstToAnalysis;

    Dictionary<ImmutableHashedString, List<IRInst*>> m_mapMangledNameToGlobalInst;
};


struct InstWorkList
{
    List<IRInst*>* workList = nullptr;
    ContainerPool* pool = nullptr;

    InstWorkList() = default;
    InstWorkList(IRModule* module)
    {
        pool = &module->getContainerPool();
        workList = module->getContainerPool().getList<IRInst>();
    }
    ~InstWorkList()
    {
        if (pool)
            pool->free(workList);
    }
    InstWorkList(const InstWorkList&) = delete;
    InstWorkList(InstWorkList&& other)
    {
        workList = other.workList;
        pool = other.pool;
        other.workList = nullptr;
        other.pool = nullptr;
    }
    InstWorkList& operator=(InstWorkList&& other)
    {
        workList = other.workList;
        pool = other.pool;
        other.workList = nullptr;
        other.pool = nullptr;
        return *this;
    }
    List<IRInst*>& getList() { return *workList; }
    IRInst* operator[](Index i) { return (*workList)[i]; }
    Index getCount() { return workList->getCount(); }
    IRInst** begin() { return workList->begin(); }
    IRInst** end() { return workList->end(); }
    IRInst* getLast() { return workList->getLast(); }
    void removeLast() { workList->removeLast(); }
    void remove(IRInst* val) { workList->remove(val); }
    void fastRemoveAt(Index index) { workList->fastRemoveAt(index); }
    void add(IRInst* inst) { workList->add(inst); }
    void clear() { workList->clear(); }
    void setCount(Index count) { workList->setCount(count); }
};

struct InstHashSet
{
    HashSet<IRInst*>* set = nullptr;
    ContainerPool* pool = nullptr;

    InstHashSet() = default;
    InstHashSet(IRModule* module)
    {
        pool = &module->getContainerPool();
        set = module->getContainerPool().getHashSet<IRInst>();
    }
    ~InstHashSet()
    {
        if (pool)
            pool->free(set);
    }
    InstHashSet(const InstHashSet&) = delete;
    InstHashSet(InstHashSet&& other)
    {
        set = other.set;
        pool = other.pool;
        other.set = nullptr;
        other.pool = nullptr;
    }
    InstHashSet& operator=(InstHashSet&& other)
    {
        set = other.set;
        pool = other.pool;
        other.set = nullptr;
        other.pool = nullptr;
        return *this;
    }
    HashSet<IRInst*>& getHashSet() { return *set; }
    Index getCount() { return set->getCount(); }
    bool add(IRInst* inst) { return set->add(inst); }
    bool contains(IRInst* inst) { return set->contains(inst); }
    void remove(IRInst* inst) { set->remove(inst); }
    void clear() { set->clear(); }
};


FIDDLE()
struct IRSpecializationDictionaryItem : public IRInst
{
    FIDDLE(leafInst())
};

struct IRDumpOptions
{
    typedef uint32_t Flags;
    struct Flag
    {
        enum Enum : Flags
        {
            SourceLocations = 0x1, ///< If set will output source locations
            DumpDebugIds = 0x2,    ///< If set *and* debug build will write ids
        };
    };

    /// How much detail to include in dumped IR.
    ///
    /// Used with the `dumpIR` functions to determine
    /// whether a completely faithful, but verbose, IR
    /// dump is produced, or something simplified for ease
    /// or reading.
    ///
    enum class Mode
    {
        /// Produce a simplified IR dump.
        ///
        /// Simplified IR dumping will skip certain instructions
        /// and print them at their use sites instead, so that
        /// the overall dump is shorter and easier to read.
        Simplified,

        /// Produce a detailed/accurate IR dump.
        ///
        /// A detailed IR dump will make sure to emit exactly
        /// the instructions that were present with no attempt
        /// to selectively skip them or give special formatting.
        ///
        Detailed,
    };

    Mode mode = Mode::Simplified;
    /// Flags to control output
    /// Add Flag::SourceLocations to output source locations set on IR
    Flags flags = 0;
};

void printSlangIRAssembly(
    StringBuilder& builder,
    IRModule* module,
    const IRDumpOptions& options,
    SourceManager* sourceManager);
String getSlangIRAssembly(
    IRModule* module,
    const IRDumpOptions& options,
    SourceManager* sourceManager);

void dumpIR(
    IRModule* module,
    const IRDumpOptions& options,
    SourceManager* sourceManager,
    ISlangWriter* writer);
void dumpIR(
    IRInst* globalVal,
    const IRDumpOptions& options,
    SourceManager* sourceManager,
    ISlangWriter* writer);
void dumpIR(
    IRModule* module,
    const IRDumpOptions& options,
    char const* label,
    SourceManager* sourceManager,
    ISlangWriter* writer);

/// True if the op type can be handled 'nominally' meaning that pointer identity is applicable.
bool isNominalOp(IROp op);

// True if the IR inst represents a builtin object (e.g. __BuiltinFloatingPointType).
bool isBuiltin(IRInst* inst);

// Get the enclosuing function of an instruction.
IRFunc* getParentFunc(IRInst* inst);

// Is child a descendent of inst
bool hasDescendent(IRInst* inst, IRInst* child);

// True if moving this inst will not change the semantics of the program
bool isMovableInst(IRInst* inst);

#if SLANG_ENABLE_IR_BREAK_ALLOC
uint32_t& _debugGetIRAllocCounter();
#endif

// TODO: Ellie, comment and move somewhere more appropriate?

template<typename I = IRInst, typename F>
static void traverseUsers(IRInst* inst, F f)
{
    List<IRUse*> uses;
    for (auto use = inst->firstUse; use; use = use->nextUse)
    {
        uses.add(use);
    }
    for (auto u : uses)
    {
        if (u->usedValue != inst)
            continue;
        if (auto s = as<I>(u->getUser()))
        {
            f(s);
        }
    }
}

template<typename F>
static void traverseUses(IRInst* inst, F f)
{
    List<IRUse*> uses;
    for (auto use = inst->firstUse; use; use = use->nextUse)
    {
        uses.add(use);
    }
    for (auto u : uses)
    {
        if (u->usedValue != inst)
            continue;
        f(u);
    }
}

namespace detail
{
// A helper to get the singular pointer argument of something callable
// Use std::function to allow passing in anything from which std::function can
// be deduced (pointers, lambdas, functors):
// https://en.cppreference.com/w/cpp/utility/functional/function/deduction_guides
// argType<T> matches T against R(A*) and returns A
template<typename R, typename A>
static A argType(std::function<R(A*)>);

// Get the class type from a pointer to member function
template<typename R, typename T>
static T thisArg(R (T::* && ())());
} // namespace detail

// A tool to "pattern match" an instruction against multiple cases
// Use like:
//
// ```
// auto r = instMatch_(myInst,
//   default,
//   [](IRStore* store){ return handleStore... },
//   [](IRType* type){ return handleTypes... },
//   );
// ```
//
// This version returns default if none of the cases match
template<typename R, typename F, typename... Fs>
R instMatch(IRInst* i, R def, F f, Fs... fs)
{
    // Recursive case
    using P = decltype(detail::argType(std::function{std::declval<F>()}));
    if (auto s = as<P>(i))
    {
        return f(s);
    }
    return instMatch(i, def, fs...);
}

// Base case with no eliminators, return the default value
template<typename R>
R instMatch(IRInst*, R def)
{
    return def;
}

// A tool to "pattern match" an instruction against multiple cases
// Use like:
//
// ```
// instMatch_(myInst,
//   [](IRStore* store){ handleStore... },
//   [](IRType* type){ handleTypes... },
//   [](IRInst* inst){ catch-all case...}
//   );
// ```
//
// This version returns nothing
template<typename F, typename... Fs>
void instMatch_(IRInst* i, F f, Fs... fs)
{
    // Recursive case
    using P = decltype(detail::argType(std::function{std::declval<F>()}));
    if (auto s = as<P>(i))
    {
        return f(s);
    }
    return instMatch_(i, fs...);
}

template<typename... Fs>
void instMatch_(IRInst*)
{
    // Base case with no eliminators
}

// A tool to compose a bunch of downcasts and accessors
// `composeGetters<R>(x, &MyStruct::getFoo, &MyOtherStruct::getBar)` translates to
// `if(auto y = as<MyStruct>) if(auto z = as<MyOtherStruct>(y->getFoo())) return as<R>(z->getBar())`
template<typename R, typename T, typename F, typename... Fs>
R* composeGetters(T* t, F f, Fs... fs)
{
    using D = decltype(detail::thisArg(std::declval<F>));
    if (D* d = as<D>(t))
    {
        auto* n = std::invoke(f, d);
        return composeGetters<R>(n, fs...);
    }
    return nullptr;
}

template<typename R, typename T>
R* composeGetters(T* t)
{
    return as<R>(t);
}

} // namespace Slang
