// slang-ir.h
#ifndef SLANG_IR_H_INCLUDED
#define SLANG_IR_H_INCLUDED

// This file defines the intermediate representation (IR) used for Slang
// shader code. This is a typed static single assignment (SSA) IR,
// similar in spirit to LLVM (but much simpler).
//

#include "../core/slang-basic.h"
#include "../core/slang-memory-arena.h"

#include "../compiler-core/slang-source-loc.h"

#include "slang-type-system-shared.h"

namespace Slang {

class   Decl;
class   GenericDecl;
class   FuncType;
class   Layout;
class   Type;
class   Session;
class   Name;
struct  IRBuilder;
struct  IRFunc;
struct  IRGlobalValueWithCode;
struct  IRInst;
struct  IRModule;

typedef unsigned int IROpFlags;
enum : IROpFlags
{
    kIROpFlags_None = 0,
    kIROpFlag_Parent = 1 << 0,                  ///< This op is a parent op
    kIROpFlag_UseOther = 1 << 1,                ///< If set this op can use 'other bits' to store information
};

/* Bit usage of IROp is a follows

          MainOp | Other
Bit range: 0-7   | Remaining bits

For doing range checks (for example for doing isa tests), the value is masked by kIROpMeta_OpMask, such that the Other bits don't interfere.
The other bits can be used for storage for anything that needs to identify as a different 'op' or 'type'. It is currently 
used currently for storing the TextureFlavor of a IRResourceTypeBase derived types for example. 

TODO: We should eliminate the use of the "other" bits so that the entire value/state
of an instruction is manifest in its opcode, operands, and children.
*/
enum IROp : int32_t
{
#define INST(ID, MNEMONIC, ARG_COUNT, FLAGS)  \
    kIROp_##ID,
#include "slang-ir-inst-defs.h"

        /// The total number of valid opcodes
    kIROpCount,

        /// An invalid opcode used to represent a missing or unknown opcode value.
    kIROp_Invalid = kIROpCount,

#define INST(ID, MNEMONIC, ARG_COUNT, FLAGS) /* empty */
#define INST_RANGE(BASE, FIRST, LAST)       \
    kIROp_First##BASE   = kIROp_##FIRST,    \
    kIROp_Last##BASE    = kIROp_##LAST,

#include "slang-ir-inst-defs.h"
};

/* IROpMeta describe values for layout of IROp, as well as values for accessing aspects of IROp bits. */
enum IROpMeta
{
    kIROpMeta_OtherShift = 10,   ///< Number of bits for op (shift right by this to get the other bits)
    kIROpMeta_OpMask = 0x3ff,    ///< Mask for just opcode
};

IROp findIROp(const UnownedStringSlice& name);

// A logical operation/opcode in the IR
struct IROpInfo
{
    // What is the name/mnemonic for this operation
    char const*     name;

    // How many required arguments are there
    // (not including the mandatory type argument)
    unsigned int    fixedArgCount;

    // Flags to control how we emit additional info
    IROpFlags       flags;
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
    IRUse*  nextUse = nullptr;

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
    IRInstListBase()
    {}

    IRInstListBase(IRInst* first, IRInst* last)
        : first(first)
        , last(last)
    {}



    IRInst* first = nullptr;
    IRInst* last = nullptr;

    IRInst* getFirst() { return first; }
    IRInst* getLast() { return last; }

    struct Iterator
    {
        IRInst* inst;

        Iterator() : inst(nullptr) {}
        Iterator(IRInst* inst) : inst(inst) {}

        void operator++();
        IRInst* operator*()
        {
            return inst;
        }

        bool operator!=(Iterator const& i)
        {
            return inst != i.inst;
        }
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
    {}

    explicit IRInstList(IRInstListBase const& list)
        : IRInstListBase(list)
    {}

    T* getFirst() { return (T*) first; }
    T* getLast() { return (T*) last; }

    struct Iterator : public IRInstListBase::Iterator
    {
        Iterator() {}
        Iterator(IRInst* inst) : IRInstListBase::Iterator(inst) {}

        T* operator*()
        {
            return (T*) inst;
        }
    };

    Iterator begin() { return Iterator(first); }
    Iterator end();
};

    /// A list of contiguous operands that can be iterated over as `IRInst`s.
struct IROperandListBase
{
    IROperandListBase()
        : m_begin(nullptr)
        , m_end(nullptr)
    {}

    IROperandListBase(
        IRUse* begin,
        IRUse* end)
        : m_begin(begin)
        , m_end(end)
    {}

    struct Iterator
    {
    public:
        Iterator()
            : m_cursor(nullptr)
        {}

        Iterator(IRUse* use)
            : m_cursor(use)
        {}

        IRInst* operator*() const
        {
            return m_cursor->get();
        }

        IRUse* getCursor() const { return m_cursor; }

        void operator++()
        {
            m_cursor++;
        }

        bool operator!=(Iterator const& that) const
        {
            return m_cursor != that.m_cursor;
        }

    protected:
        IRUse* m_cursor;
    };

    Iterator begin() const { return Iterator(m_begin); }
    Iterator end() const { return Iterator(m_end); }

    Int getCount() const { return m_end - m_begin; }

    IRInst* operator[](Int index) const
    {
        return m_begin[index].get();
    }

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
    IROperandList()
    {}

    IROperandList(
        IRUse* begin,
        IRUse* end)
        : Super(begin, end)
    {}

    struct Iterator : public IROperandListBase::Iterator
    {
        typedef IROperandListBase::Iterator Super;
    public:
        Iterator()
        {}

        Iterator(IRUse* use)
            : Super(use)
        {}

        T* operator*() const
        {
            return (T*) m_cursor->get();
        }
    };

    Iterator begin() const { return Iterator(m_begin); }
    Iterator end() const { return Iterator(m_end); }

    T* operator[](Int index) const
    {
        return (T*) m_begin[index].get();
    }
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
        None,       //< Don't insert new instructions at all; just create them
        Before,     //< Insert immediately before the existing instruction
        After,      //< Insert immediately after the existing instruction
        AtStart,    //< Insert at the start of the existing instruction's child list
        AtEnd,      //< Insert at the start of the existing instruction's child list
    };

        /// Construct a default insertion location in the `None` mode.
    IRInsertLoc()
    {}

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

        /// Get the enclosing function (or other code-bearing value) that instructions are inserted into.
        ///
        /// This searches up the parent chain starting with `getParent()` looking for a code-bearing
        /// value that things are being inserted into (could be a function, generic, etc.)
        ///
    IRGlobalValueWithCode* getFunc() const;

private:
        /// Internal constructor
    IRInsertLoc(Mode mode, IRInst* inst)
        : m_mode(mode)
        , m_inst(inst)
    {}

        /// The insertion mode
    Mode m_mode = Mode::None;

        /// The instruction that insertions will be made relative to.
        ///
        /// Should always be null for the `None` mode and non-null for all other modes.
    IRInst* m_inst = nullptr;
};

// Every value in the IR is an instruction (even things
// like literal values).
//
struct IRInst
{
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

    UInt getOperandCount()
    {
        return operandCount;
    }

    // Source location information for this value, if any
    SourceLoc sourceLoc;

    // Each instruction can have zero or more "decorations"
    // attached to it. A decoration is a specialized kind
    // of instruction that either attaches metadata to,
    // or modifies the sematnics of, its parent instruction.
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
        for( auto attr : getAllAttrs() )
        {
            if(auto found = as<T>(attr))
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
        while(bb != end && !as<T>(*bb))
            ++bb;
        auto ee = bb;
        while(ee != end && as<T>(*ee))
            ++ee;
        return IROperandList<T>(bb.getCursor(),ee.getCursor());
    }

    // The first use of this value (start of a linked list)
    IRUse*      firstUse = nullptr;


    // The parent of this instruction.
    IRInst*   parent;

    IRInst* getParent() { return parent; }

    // The next and previous instructions with the same parent
    IRInst*         next;
    IRInst*         prev;

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
    IRInstList<IRInst> getChildren()
    {
        return IRInstList<IRInst>(
            getFirstChild(),
            getLastChild());
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
    IRInst* getLastDecorationOrChild()  { return m_decorationsAndChildren.last;  }
    IRInstListBase getDecorationsAndChildren() { return m_decorationsAndChildren; }

    void removeAndDeallocateAllDecorationsAndChildren();

#ifdef SLANG_ENABLE_IR_BREAK_ALLOC
    // Unique allocation ID for this instruction since start of current process.
    // Used to aid debugging only.
    uint32_t _debugUID;
#endif

    // The type of the result value of this instruction,
    // or `null` to indicate that the instruction has
    // no value.
    IRUse typeUse;

    IRType* getFullType() { return (IRType*) typeUse.get(); }
    void setFullType(IRType* type) { typeUse.init(this, (IRInst*) type); }

    IRRate* getRate();

    IRType* getDataType();

    // After the type, we have data that is specific to
    // the subtype of `IRInst`. In most cases, this is
    // just a series of `IRUse` values representing
    // operands of the instruction.

    IRUse*      getOperands();

    IRInst* getOperand(UInt index)
    {
        return getOperands()[index].get();
    }

    void setOperand(UInt index, IRInst* value)
    {
        SLANG_ASSERT(getOperands()[index].user != nullptr);
        getOperands()[index].set(value);
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

        /// Transfer any decorations of this instruction to the `target` instruction.
    void transferDecorationsTo(IRInst* target);

    /// Does this instruction have any uses?
    bool hasUses() const { return firstUse != nullptr; }

    /// Does this instructiomn have more than one use?
    bool hasMoreThanOneUse() const { return firstUse != nullptr && firstUse->nextUse != nullptr; }

    /// It is possible that this instruction has side effects?
    ///
    /// This is a conservative test, and will return `true` if an exact answer can't be determined.
    bool mightHaveSideEffects();

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
        /// If `inPrev` is non-null it must satisfy `inPrev->getNextInst() == inNext` and `inPrev->getParent() == inParent`
        /// If `inNext` is non-null it must satisfy `inNext->getPrevInst() == inPrev` and `inNext->getParent() == inParent`
        ///
        /// If both `inPrev` and `inNext` are null, then `inParent` must have no (raw) children.
        ///
    void _insertAt(IRInst* inPrev, IRInst* inNext, IRInst* inParent);
};

template<typename T>
T* dynamicCast(IRInst* inst)
{
    if (inst && T::isaImpl(inst->getOp()))
        return static_cast<T*>(inst);
    return nullptr;
}

template<typename T>
const T* dynamicCast(const IRInst* inst)
{
    if (inst && T::isaImpl(inst->getOp()))
        return static_cast<const T*>(inst);
    return nullptr;
}

// `dynamic_cast` equivalent (we just use dynamicCast)
template<typename T>
T* as(IRInst* inst)
{
    return dynamicCast<T>(inst);
}

template<typename T>
const T* as(const IRInst* inst)
{
    return dynamicCast<T>(inst);
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
    for( auto decoration : getDecorations() )
    {
        if(auto match = as<T>(decoration))
            return match;
    }
    return nullptr;
}

template<typename T>
typename IRInstList<T>::Iterator IRInstList<T>::end()
{
    return Iterator(last ? last->next : nullptr);
}


// Types

#define IR_LEAF_ISA(NAME) static bool isaImpl(IROp op) { return (kIROpMeta_OpMask & op) == kIROp_##NAME; }
#define IR_PARENT_ISA(NAME) static bool isaImpl(IROp opIn) { const int op = (kIROpMeta_OpMask & opIn); return op >= kIROp_First##NAME && op <= kIROp_Last##NAME; }

#define SIMPLE_IR_TYPE(NAME, BASE) struct IR##NAME : IR##BASE { IR_LEAF_ISA(NAME) };
#define SIMPLE_IR_PARENT_TYPE(NAME, BASE) struct IR##NAME : IR##BASE { IR_PARENT_ISA(NAME) };


// All types in the IR are represented as instructions which conceptually
// execute before run time.
struct IRType : IRInst
{
    IRType* getCanonicalType() { return this; }

    // Hack: specialize can also be a type. We should consider using a
    // separate `specializeType` op code for types so we can use the normal
    // `IR_PARENT_ISA` macro here.
    static bool isaImpl(IROp opIn)
    {
        const int op = (kIROpMeta_OpMask & opIn);
        return (op >= kIROp_FirstType && op <= kIROp_LastType) || op == kIROp_Specialize;
    }
};

IRType* unwrapArray(IRType* type);

struct IRBasicType : IRType
{
    BaseType getBaseType() { return BaseType(getOp() - kIROp_FirstBasicType); }

    IR_PARENT_ISA(BasicType)
};

struct IRVoidType : IRBasicType
{
    IR_LEAF_ISA(VoidType)
};

struct IRBoolType : IRBasicType
{
    IR_LEAF_ISA(BoolType)
};

struct IRStringTypeBase : IRType
{
    IR_PARENT_ISA(StringTypeBase)
};

SIMPLE_IR_TYPE(StringType, StringTypeBase)
SIMPLE_IR_TYPE(NativeStringType, StringTypeBase)

SIMPLE_IR_TYPE(DynamicType, Type)

// True if types are equal
// Note compares nominal types by name alone 
bool isTypeEqual(IRType* a, IRType* b);

void findAllInstsBreadthFirst(IRInst* inst, List<IRInst*>& outInsts);

// Constant Instructions

typedef int64_t IRIntegerValue;
typedef double IRFloatingPointValue;

struct IRConstant : IRInst
{
    enum class FloatKind
    {
        Finite,
        PositiveInfinity,
        NegativeInfinity,
        Nan,
    };

    struct StringValue
    {   
        uint32_t numChars;           ///< The number of chars
        char chars[1];               ///< Chars added at end. NOTE! Must be last member of struct! 
    };
    struct StringSliceValue
    {
        uint32_t numChars;
        char* chars;
    };

    union ValueUnion
    {
        IRIntegerValue          intVal;         ///< Used for integrals and boolean
        IRFloatingPointValue    floatVal;
        void*                   ptrVal;

        /// Either of these types could be set with kIROp_StringLit. 
        /// Which is used is currently determined with decorations - if a kIROp_TransitoryDecoration is set, then the transitory StringVal is used, else stringVal
        // which relies on chars being held after the struct).
        StringValue             stringVal;
        StringSliceValue        transitoryStringVal;           
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

    IR_PARENT_ISA(Constant)

    // Must be last member, because data may be held behind
    // NOTE! The total size of IRConstant may not be allocated - only enough space is allocated for the value type held in the union.
    ValueUnion value;
};

struct IRIntLit : IRConstant
{
    IRIntegerValue getValue() { return value.intVal; }

    IR_LEAF_ISA(IntLit);
};

struct IRBoolLit : IRConstant
{
    bool getValue() { return value.intVal != 0; }

    IR_LEAF_ISA(BoolLit);
};

// Get the compile-time constant integer value of an instruction,
// if it has one, and assert-fail otherwise.
IRIntegerValue getIntVal(IRInst* inst);

struct IRStringLit : IRConstant
{
    
    IR_LEAF_ISA(StringLit);
};

struct IRPtrLit : IRConstant
{
    IR_LEAF_ISA(PtrLit);

    void* getValue() { return value.ptrVal; }
};

struct IRVoidLit : IRConstant
{
    IR_LEAF_ISA(VoidLit);
};

// A instruction that ends a basic block (usually because of control flow)
struct IRTerminatorInst : IRInst
{
    IR_PARENT_ISA(TerminatorInst)
};

// A function parameter is owned by a basic block, and represents
// either an incoming function parameter (in the entry block), or
// a value that flows from one SSA block to another (in a non-entry
// block).
//
// In each case, the basic idea is that a block is a "label with
// arguments."
//
struct IRParam : IRInst
{
    IRParam* getNextParam();
    IRParam* getPrevParam();

    IR_LEAF_ISA(Param)
};

    /// A control-flow edge from one basic block to another
struct IREdge
{
public:
    IREdge()
    {}

    explicit IREdge(IRUse* use)
        : m_use(use)
    {}

    IRBlock* getPredecessor() const;
    IRBlock* getSuccessor() const;

    IRUse* getUse() const
    {
        return m_use;
    }

    bool isCritical() const;

private:
    IRUse* m_use = nullptr;
};

// A basic block is a parent instruction that adds the constraint
// that all the children need to be "ordinary" instructions (so
// no function declarations, or nested blocks). We also expect
// that the previous/next instruction are always a basic block.
//
struct IRBlock : IRInst
{
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
    IRParam* getFirstParam() { return as<IRParam>(getFirstInst()); }
    IRParam* getLastParam();
    IRInstList<IRParam> getParams()
    {
        return IRInstList<IRParam>(
            getFirstParam(),
            getLastParam());
    }

    void addParam(IRParam* param);
    void insertParamAtHead(IRParam* param);

    // The "ordinary" instructions come after the parameters
    IRInst* getFirstOrdinaryInst();
    IRInst* getLastOrdinaryInst();
    IRInstList<IRInst> getOrdinaryInsts()
    {
        return IRInstList<IRInst>(
            getFirstOrdinaryInst(),
            getLastOrdinaryInst());
    }

    // The parent of a basic block is assumed to be a
    // value with code (e.g., a function, global variable
    // with initializer, etc.).
    IRGlobalValueWithCode* getParent() { return cast<IRGlobalValueWithCode>(IRInst::getParent()); }

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
        PredecessorList(IRUse* begin) : b(begin) {}
        IRUse* b;

        UInt getCount();
        bool isEmpty();

        struct Iterator
        {
            Iterator(IRUse* use) : use(use) {}

            IRBlock* operator*();

            void operator++();

            bool operator!=(Iterator const& that)
            {
                return use != that.use;
            }

            IREdge getEdge() const { return IREdge(use); }
            IRUse* use;
        };

        Iterator begin() { return Iterator(b); }
        Iterator end()   { return Iterator(nullptr); }
    };

    struct SuccessorList
    {
        SuccessorList(IRUse* begin, IRUse* end, UInt stride = 1) : begin_(begin), end_(end), stride(stride) {}
        IRUse* begin_;
        IRUse* end_;
        UInt stride;

        UInt getCount();

        struct Iterator
        {
            Iterator(IRUse* use, UInt stride) : use(use), stride(stride) {}

            IRBlock* operator*();

            void operator++();

            bool operator!=(Iterator const& that)
            {
                return use != that.use;
            }

            IREdge getEdge() const { return IREdge(use); }

            IRUse* use;
            UInt stride;
        };

        Iterator begin() { return Iterator(begin_, stride); }
        Iterator end()   { return Iterator(end_, stride); }
    };

    PredecessorList getPredecessors();
    SuccessorList getSuccessors();

    //

    IR_LEAF_ISA(Block)
};

SIMPLE_IR_TYPE(BasicBlockType, Type)

struct IRResourceTypeBase : IRType
{
    TextureFlavor getFlavor() const
    {
        return TextureFlavor((getOp() >> kIROpMeta_OtherShift) & 0xFFFF);
    }

    TextureFlavor::Shape GetBaseShape() const
    {
        return getFlavor().getBaseShape();
    }
    bool isFeedback() const { return getFlavor().isFeedback(); }
    bool isMultisample() const { return getFlavor().isMultisample(); }
    bool isArray() const { return getFlavor().isArray(); }
    SlangResourceShape getShape() const { return getFlavor().getShape(); }
    SlangResourceAccess getAccess() const { return getFlavor().getAccess(); }

    IR_PARENT_ISA(ResourceTypeBase);
};

struct IRResourceType : IRResourceTypeBase
{
    IRType* getElementType() { return (IRType*)getOperand(0); }

    IR_PARENT_ISA(ResourceType)
};

struct IRTextureTypeBase : IRResourceType
{
    IR_PARENT_ISA(TextureTypeBase)
};

struct IRTextureType : IRTextureTypeBase
{
    IR_LEAF_ISA(TextureType)
};

struct IRTextureSamplerType : IRTextureTypeBase
{
    IR_LEAF_ISA(TextureSamplerType)
};

struct IRGLSLImageType : IRTextureTypeBase
{
    IR_LEAF_ISA(GLSLImageType)
};

struct IRSamplerStateTypeBase : IRType
{
    IR_PARENT_ISA(SamplerStateTypeBase)
};

SIMPLE_IR_TYPE(SamplerStateType, SamplerStateTypeBase)
SIMPLE_IR_TYPE(SamplerComparisonStateType, SamplerStateTypeBase)

struct IRBuiltinGenericType : IRType
{
    IRType* getElementType() { return (IRType*)getOperand(0); }

    IR_PARENT_ISA(BuiltinGenericType)
};

SIMPLE_IR_PARENT_TYPE(PointerLikeType, BuiltinGenericType);
SIMPLE_IR_PARENT_TYPE(HLSLStructuredBufferTypeBase, BuiltinGenericType)
SIMPLE_IR_TYPE(HLSLStructuredBufferType, HLSLStructuredBufferTypeBase)
SIMPLE_IR_TYPE(HLSLRWStructuredBufferType, HLSLStructuredBufferTypeBase)
SIMPLE_IR_TYPE(HLSLRasterizerOrderedStructuredBufferType, HLSLStructuredBufferTypeBase)

SIMPLE_IR_PARENT_TYPE(UntypedBufferResourceType, Type)
SIMPLE_IR_PARENT_TYPE(ByteAddressBufferTypeBase, UntypedBufferResourceType)
SIMPLE_IR_TYPE(HLSLByteAddressBufferType, ByteAddressBufferTypeBase)
SIMPLE_IR_TYPE(HLSLRWByteAddressBufferType, ByteAddressBufferTypeBase)
SIMPLE_IR_TYPE(HLSLRasterizerOrderedByteAddressBufferType, ByteAddressBufferTypeBase)

SIMPLE_IR_TYPE(HLSLAppendStructuredBufferType, HLSLStructuredBufferTypeBase)
SIMPLE_IR_TYPE(HLSLConsumeStructuredBufferType, HLSLStructuredBufferTypeBase)

struct IRHLSLPatchType : IRType
{
    IRType* getElementType() { return (IRType*)getOperand(0); }
    IRInst* getElementCount() { return getOperand(1); }

    IR_PARENT_ISA(HLSLPatchType)
};

SIMPLE_IR_TYPE(HLSLInputPatchType, HLSLPatchType)
SIMPLE_IR_TYPE(HLSLOutputPatchType, HLSLPatchType)

SIMPLE_IR_PARENT_TYPE(HLSLStreamOutputType, BuiltinGenericType)
SIMPLE_IR_TYPE(HLSLPointStreamType, HLSLStreamOutputType)
SIMPLE_IR_TYPE(HLSLLineStreamType, HLSLStreamOutputType)
SIMPLE_IR_TYPE(HLSLTriangleStreamType, HLSLStreamOutputType)

SIMPLE_IR_TYPE(GLSLInputAttachmentType, Type)
SIMPLE_IR_PARENT_TYPE(ParameterGroupType, PointerLikeType)
SIMPLE_IR_PARENT_TYPE(UniformParameterGroupType, ParameterGroupType)
SIMPLE_IR_PARENT_TYPE(VaryingParameterGroupType, ParameterGroupType)
SIMPLE_IR_TYPE(ConstantBufferType, UniformParameterGroupType)
SIMPLE_IR_TYPE(TextureBufferType, UniformParameterGroupType)
SIMPLE_IR_TYPE(GLSLInputParameterGroupType, VaryingParameterGroupType)
SIMPLE_IR_TYPE(GLSLOutputParameterGroupType, VaryingParameterGroupType)
SIMPLE_IR_TYPE(GLSLShaderStorageBufferType, UniformParameterGroupType)
SIMPLE_IR_TYPE(ParameterBlockType, UniformParameterGroupType)

struct IRArrayTypeBase : IRType
{
    IRType* getElementType() { return (IRType*)getOperand(0); }

    // Returns the element count for an `IRArrayType`, and null
    // for an `IRUnsizedArrayType`.
    IRInst* getElementCount();

    IR_PARENT_ISA(ArrayTypeBase)
};

struct IRArrayType: IRArrayTypeBase
{
    IRInst* getElementCount() { return getOperand(1); }

    IR_LEAF_ISA(ArrayType)
};

SIMPLE_IR_TYPE(UnsizedArrayType, ArrayTypeBase)

SIMPLE_IR_PARENT_TYPE(Rate, Type)
SIMPLE_IR_TYPE(ConstExprRate, Rate)
SIMPLE_IR_TYPE(GroupSharedRate, Rate)
SIMPLE_IR_TYPE(ActualGlobalRate, Rate)

struct IRRateQualifiedType : IRType
{
    IRRate* getRate() { return (IRRate*) getOperand(0); }
    IRType* getValueType() { return (IRType*) getOperand(1); }

    IR_LEAF_ISA(RateQualifiedType)
};


// Unlike the AST-level type system where `TypeType` tracks the
// underlying type, the "type of types" in the IR is a simple
// value with no operands, so that all type nodes have the
// same type.
SIMPLE_IR_PARENT_TYPE(Kind, Type);
SIMPLE_IR_TYPE(TypeKind, Kind);

// The kind of any and all generics.
//
// A more complete type system would include "arrow kinds" to
// be able to track the domain and range of generics (e.g.,
// the `vector` generic maps a type and an integer to a type).
// This is only really needed if we ever wanted to support
// "higher-kinded" generics (e.g., a generic that takes another
// generic as a parameter).
//
SIMPLE_IR_TYPE(GenericKind, Kind)

struct IRVectorType : IRType
{
    IRType* getElementType() { return (IRType*)getOperand(0); }
    IRInst* getElementCount() { return getOperand(1); }

    IR_LEAF_ISA(VectorType)
};

struct IRMatrixType : IRType
{
    IRType* getElementType() { return (IRType*)getOperand(0); }
    IRInst* getRowCount() { return getOperand(1); }
    IRInst* getColumnCount() { return getOperand(2); }

    IR_LEAF_ISA(MatrixType)
};

struct IRSPIRVLiteralType : IRType
{
    IR_LEAF_ISA(SPIRVLiteralType)

    IRType* getValueType() { return static_cast<IRType*>(getOperand(0)); }
};

struct IRPtrTypeBase : IRType
{
    IRType* getValueType() { return (IRType*)getOperand(0); }

    bool hasAddressSpace() { return getOperandCount() > 1; }

    IRIntegerValue getAddressSpace()
    {
        return getOperandCount() > 1 ? static_cast<IRIntLit*>(getOperand(1))->getValue() : -1;
    }

    IR_PARENT_ISA(PtrTypeBase)
};

SIMPLE_IR_TYPE(PtrType, PtrTypeBase)
SIMPLE_IR_TYPE(RefType, PtrTypeBase)
SIMPLE_IR_PARENT_TYPE(OutTypeBase, PtrTypeBase)
SIMPLE_IR_TYPE(OutType, OutTypeBase)
SIMPLE_IR_TYPE(InOutType, OutTypeBase)

SIMPLE_IR_TYPE(ComPtrType, Type)
SIMPLE_IR_TYPE(NativePtrType, Type)

struct IRPseudoPtrType : public IRPtrTypeBase
{
    IR_LEAF_ISA(PseudoPtrType);
};

    /// The base class of RawPointerType and RTTIPointerType.
struct IRRawPointerTypeBase : IRType
{
    IR_PARENT_ISA(RawPointerTypeBase);
};

    /// Represents a pointer to an object of unknown type.
struct IRRawPointerType : IRRawPointerTypeBase
{
    IR_LEAF_ISA(RawPointerType)
};

    /// Represents a pointer to an object whose type is determined at runtime,
    /// with type information available through `rttiOperand`.
    ///
struct IRRTTIPointerType : IRRawPointerTypeBase
{
    IRInst* getRTTIOperand() { return getOperand(0); }
    IR_LEAF_ISA(RTTIPointerType)
};

struct IRGlobalHashedStringLiterals : IRInst
{
    IR_LEAF_ISA(GlobalHashedStringLiterals)
};

struct IRGetStringHash : IRInst
{
    IR_LEAF_ISA(GetStringHash)

    IRStringLit* getStringLit() { return as<IRStringLit>(getOperand(0)); }
};

    /// Get the type pointed to be `ptrType`, or `nullptr` if it is not a pointer(-like) type.
    ///
    /// The given IR `builder` will be used if new instructions need to be created.
IRType* tryGetPointedToType(
        IRBuilder*  builder,
        IRType*     type);

struct IRFuncType : IRType
{
    IRType* getResultType() { return (IRType*) getOperand(0); }
    UInt getParamCount() { return getOperandCount() - 1; }
    IRType* getParamType(UInt index) { return (IRType*)getOperand(1 + index); }

    IR_LEAF_ISA(FuncType)
};

bool isDefinition(
    IRInst* inVal);

// A structure type is represented as a parent instruction,
// where the child instructions represent the fields of the
// struct.
//
// The space of fields that a given struct type supports
// are defined as its "keys", which are global values
// (that is, they have mangled names that can be used
// for linkage).
//
struct IRStructKey : IRInst
{
    IR_LEAF_ISA(StructKey)
};
//
// The fields of the struct are then defined as mappings
// from those keys to the associated type (in the case of
// the struct type) or to values (when lookup up a field).
//
// A struct field thus has two operands: the key, and the
// type of the field.
//
struct IRStructField : IRInst
{
    IRStructKey* getKey() { return cast<IRStructKey>(getOperand(0)); }
    IRType* getFieldType()
    {
        // Note: We do not use `cast` here because there are
        // cases of types (which we would like to conveniently
        // refer to via an `IRType*`) which do not actually
        // inherit from `IRType` in the hierarchy.
        //
        return (IRType*) getOperand(1);
    }

    IR_LEAF_ISA(StructField)
};
//
// The struct type is then represented as a parent instruction
// that contains the various fields. Note that a struct does
// *not* contain the keys, because code needs to be able to
// reference the keys from scopes outside of the struct.
//
struct IRStructType : IRType
{
    IRInstList<IRStructField> getFields() { return IRInstList<IRStructField>(getChildren()); }

    IR_LEAF_ISA(StructType)
};

struct IRClassType : IRType
{
    IRInstList<IRStructField> getFields() { return IRInstList<IRStructField>(getChildren()); }

    IR_LEAF_ISA(ClassType)
};

struct IRAssociatedType : IRType
{
    IR_LEAF_ISA(AssociatedType)
};

struct IRThisType : IRType
{
    IR_LEAF_ISA(ThisType)

    IRInst* getConstraintType()
    {
        return getOperand(0);
    }
};

struct IRInterfaceRequirementEntry : IRInst
{
    IRInst* getRequirementKey() { return getOperand(0); }
    IRInst* getRequirementVal() { return getOperand(1); }
    void setRequirementKey(IRInst* val) { setOperand(0, val); }
    void setRequirementVal(IRInst* val) { setOperand(1, val); }

    IR_LEAF_ISA(InterfaceRequirementEntry);
};

struct IRInterfaceType : IRType
{
    IR_LEAF_ISA(InterfaceType)
};

struct IRTaggedUnionType : IRType
{
    IR_LEAF_ISA(TaggedUnionType)
};

struct IRConjunctionType : IRType
{
    IR_LEAF_ISA(ConjunctionType)

    Int getCaseCount() { return getOperandCount(); }
    IRType* getCaseType(Int index) { return (IRType*) getOperand(index); }
};

struct IRAttributedType : IRType
{
    IR_LEAF_ISA(AttributedType)

    IRType* getBaseType() { return (IRType*) getOperand(0); }
};

/// Represents a tuple. Tuples are created by `IRMakeTuple` and its elements
/// are accessed via `GetTupleElement(tupleValue, IRIntLit)`.
struct IRTupleType : IRType
{
    IR_LEAF_ISA(TupleType)
};

/// Represents an `Result<T,E>`, used by functions that throws error codes.
struct IRResultType : IRType
{
    IR_LEAF_ISA(ResultType)

    IRType* getValueType() { return (IRType*)getOperand(0); }
    IRType* getErrorType() { return (IRType*)getOperand(1); }
};

struct IRTypeType : IRType
{
    IR_LEAF_ISA(TypeType);
};

    /// Represents the IR type for an `IRRTTIObject`.
struct IRRTTIType : IRType
{
    IR_LEAF_ISA(RTTIType);
};

    /// Represents a handle to an RTTI object.
    /// This is lowered as an integer number identifying a type.
struct IRRTTIHandleType : IRType
{
    IR_LEAF_ISA(RTTIHandleType);
};

struct IRAnyValueType : IRType
{
    IR_LEAF_ISA(AnyValueType);
    IRInst* getSize()
    {
        return getOperand(0);
    }
};

struct IRWitnessTableTypeBase : IRType
{
    IRInst* getConformanceType() { return getOperand(0); }
    IR_PARENT_ISA(WitnessTableTypeBase);
};

struct IRWitnessTableType : IRWitnessTableTypeBase
{
    IR_LEAF_ISA(WitnessTableType);
};

struct IRWitnessTableIDType : IRWitnessTableTypeBase
{
    IR_LEAF_ISA(WitnessTableIDType);
};

struct IRBindExistentialsTypeBase : IRType
{
    IR_PARENT_ISA(BindExistentialsTypeBase)

    IRType* getBaseType() { return (IRType*) getOperand(0); }
    UInt getExistentialArgCount() { return getOperandCount() - 1; }
    IRUse* getExistentialArgs() { return getOperands() + 1; }
    IRInst* getExistentialArg(UInt index) { return getExistentialArgs()[index].get(); }
};

struct IRBindExistentialsType : IRBindExistentialsTypeBase
{
    IR_LEAF_ISA(BindExistentialsType)

};

struct IRBoundInterfaceType : IRBindExistentialsTypeBase
{
    IR_LEAF_ISA(BoundInterfaceType)

    IRType* getInterfaceType() { return getBaseType(); }
    IRType* getConcreteType() { return (IRType*) getExistentialArg(0); }
    IRInst* getWitnessTable() { return getExistentialArg(1); }
};


/// @brief A global value that potentially holds executable code.
///
struct IRGlobalValueWithCode : IRInst
{
    // The children of a value with code will be the basic
    // blocks of its definition.
    IRBlock* getFirstBlock() { return cast<IRBlock>(getFirstChild()); }
    IRBlock* getLastBlock() { return cast<IRBlock>(getLastChild()); }
    IRInstList<IRBlock> getBlocks()
    {
        return IRInstList<IRBlock>(getChildren());
    }

    // Add a block to the end of this function.
    void addBlock(IRBlock* block);

    IR_PARENT_ISA(GlobalValueWithCode)
};

// A value that has parameters so that it can conceptually be called.
struct IRGlobalValueWithParams : IRGlobalValueWithCode
{
    // Convenience accessor for the IR parameters,
    // which are actually the parameters of the first
    // block.
    IRParam* getFirstParam();
    IRParam* getLastParam();
    IRInstList<IRParam> getParams();
    IRInst* getFirstOrdinaryInst();

    IR_PARENT_ISA(GlobalValueWithParams)
};

// A function is a parent to zero or more blocks of instructions.
//
// A function is itself a value, so that it can be a direct operand of
// an instruction (e.g., a call).
struct IRFunc : IRGlobalValueWithParams
{
    // The type of the IR-level function
    IRFuncType* getDataType() { return (IRFuncType*) IRInst::getDataType(); }

    // Convenience accessors for working with the
    // function's type.
    IRType* getResultType();
    UInt getParamCount();
    IRType* getParamType(UInt index);

    bool isDefinition() { return getFirstBlock() != nullptr; }

    IR_LEAF_ISA(Func)
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
struct IRGeneric : IRGlobalValueWithParams
{
    IR_LEAF_ISA(Generic)
};

// Find the value that is returned from a generic, so that
// a pass can glean information from it.
IRInst* findGenericReturnVal(IRGeneric* generic);
// Recursively find the inner most generic return value.
IRInst* findInnerMostGenericReturnVal(IRGeneric* generic);

struct IRSpecialize;
IRGeneric* findSpecializedGeneric(IRSpecialize* specialize);
IRInst* findSpecializeReturnVal(IRSpecialize* specialize);

// Resolve an instruction that might reference a static definition
// to the most specific IR node possible, so that we can read
// decorations from it (e.g., if this is a `specialize` instruction,
// then try to chase down the generic being specialized, and what
// it seems to return).
//
IRInst* getResolvedInstForDecorations(IRInst* inst);

// The IR module itself is represented as an instruction, which
// serves at the root of the tree of all instructions in the module.
struct IRModuleInst : IRInst
{
    // Pointer back to the non-instruction object that represents
    // the module, so that we can get back to it in algorithms
    // that need it.
    IRModule* module;

    IRInstListBase getGlobalInsts() { return getChildren(); }

    IR_LEAF_ISA(Module)
};

struct IRModule : RefObject
{
public:
    enum 
    {
        kMemoryArenaBlockSize = 16 * 1024,           ///< Use 16k block size for memory arena
    };

    static RefPtr<IRModule> create(Session* session);

    SLANG_FORCE_INLINE Session* getSession() const { return m_session; }
    SLANG_FORCE_INLINE IRModuleInst* getModuleInst() const { return m_moduleInst;  }
    SLANG_FORCE_INLINE MemoryArena& getMemoryArena() { return m_memoryArena; }

    IRInstListBase getGlobalInsts() const { return getModuleInst()->getChildren(); }

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
    IRInst* _allocateInst(
        IROp    op,
        Int     operandCount,
        size_t  minSizeInBytes = 0);

    template<typename T>
    T* _allocateInst(
        IROp    op,
        Int     operandCount)
    {
        return (T*) _allocateInst(op, operandCount, sizeof(T));
    }

private:
    IRModule() = delete;

        /// Ctor
    IRModule(Session* session)
        : m_session(session)
        , m_memoryArena(kMemoryArenaBlockSize)
    {
    }

        // The compilation session in use.
    Session*    m_session = nullptr;

        /// The root IR instruction for the module.
        ///
        /// All other IR instructions that make up the state/contents of the module are
        /// descendents of this instruction. Thus if we follow the chain of parent
        /// instructions from an arbitrary IR instruction we expect to find the
        /// `IRModuleInst` for the module the instruction belongs to, if any.
        ///
    IRModuleInst* m_moduleInst = nullptr;

        /// The memory arena from which all IR instructions (and any associated state) in this module are allocated.
    MemoryArena m_memoryArena;
};

struct IRSpecializationDictionaryItem : public IRInst
{
    IR_LEAF_ISA(SpecializationDictionaryItem)
};

struct IRDumpOptions
{
    typedef uint32_t Flags;
    struct Flag
    {
        enum Enum : Flags
        {
            SourceLocations = 0x1,          ///< If set will output source locations
            DumpDebugIds    = 0x2,          ///< If set *and* debug build will write ids
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

void printSlangIRAssembly(StringBuilder& builder, IRModule* module, const IRDumpOptions& options, SourceManager* sourceManager);
String getSlangIRAssembly(IRModule* module, const IRDumpOptions& options, SourceManager* sourceManager);

void dumpIR(IRModule* module, const IRDumpOptions& options, SourceManager* sourceManager, ISlangWriter* writer);
void dumpIR(IRInst* globalVal, const IRDumpOptions& options, SourceManager* sourceManager, ISlangWriter* writer);
void dumpIR(IRModule* module, const IRDumpOptions& options, char const* label, SourceManager* sourceManager, ISlangWriter* writer);

    /// True if the op type can be handled 'nominally' meaning that pointer identity is applicable. 
bool isNominalOp(IROp op);

    // True if the IR inst represents a builtin object (e.g. __BuiltinFloatingPointType).
bool isBuiltin(IRInst* inst);

    // Get the enclosuing function of an instruction.
IRFunc* getParentFunc(IRInst* inst);

#if SLANG_ENABLE_IR_BREAK_ALLOC
uint32_t& _debugGetIRAllocCounter();
#endif

}

#endif
