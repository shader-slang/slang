// ir.h
#ifndef SLANG_IR_H_INCLUDED
#define SLANG_IR_H_INCLUDED

// This file defines the intermediate representation (IR) used for Slang
// shader code. This is a typed static single assignment (SSA) IR,
// similar in spirit to LLVM (but much simpler).
//

#include "../core/basic.h"

#include "source-loc.h"
#include "memory_pool.h"

namespace Slang {

class   Decl;
class   GenericDecl;
class   FuncType;
class   Layout;
class   Type;
class   Session;
class   Name;
struct  IRFunc;
struct  IRGlobalValueWithCode;
struct  IRInst;
struct  IRModule;

typedef unsigned int IROpFlags;
enum : IROpFlags
{
    kIROpFlags_None = 0,

    // This op is a parent op
    kIROpFlag_Parent = 1 << 0,
};

enum IROp : int16_t
{
#define INST(ID, MNEMONIC, ARG_COUNT, FLAGS)  \
    kIROp_##ID,

#include "ir-inst-defs.h"

    kIROpCount,

    // We use the negative range of opcode values
    // to encode "pseudo" instructions that should
    // not appear in valid IR.

    kIRPseduoOp_FirstPseudo = -1000,

#define INST(ID, MNEMONIC, ARG_COUNT, FLAGS) /* empty */
#define PSEUDO_INST(ID) kIRPseudoOp_##ID,

#include "ir-inst-defs.h"

#define INST(ID, MNEMONIC, ARG_COUNT, FLAGS) /* empty */
#define INST_RANGE(BASE, FIRST, LAST)       \
    kIROp_First##BASE   = kIROp_##FIRST,    \
    kIROp_Last##BASE    = kIROp_##LAST,

#include "ir-inst-defs.h"

    kIROp_Invalid = -1,

};

IROp findIROp(char const* name);

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
    IRInst* get() { return usedValue; }
    IRInst* getUser() { return user; }

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

enum IRDecorationOp : uint16_t
{
    kIRDecorationOp_HighLevelDecl,
    kIRDecorationOp_Layout,
    kIRDecorationOp_LoopControl,
    kIRDecorationOp_Target,
    kIRDecorationOp_TargetIntrinsic,
};

// represents an object allocated in an IR memory pool 
struct IRObject
{
    bool isDestroyed = false;
    virtual void dispose()
    {
        isDestroyed = true;
    }
    virtual ~IRObject()
    {
        isDestroyed = true;
    }
};

// A "decoration" that gets applied to an instruction.
// These usually don't affect semantics, but are useful
// for preserving high-level source information.
struct IRDecoration : public IRObject
{
    // Next decoration attached to the same instruction
    IRDecoration* next;

    IRDecorationOp op;
};

// Use AST-level types directly to represent the
// types of IR instructions/values
typedef Type IRType;

struct IRBlock;
struct IRParentInst;

// Every value in the IR is an instruction (even things
// like literal values).
//
struct IRInst : public IRObject
{
    // The operation that this value represents
    IROp op;

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

    // The linked list of decorations attached to this value
    IRDecoration* firstDecoration = nullptr;

    // Look up a decoration in the list of decorations
    IRDecoration* findDecorationImpl(IRDecorationOp op);
    template<typename T>
    T* findDecoration()
    {
        return (T*) findDecorationImpl(IRDecorationOp(T::kDecorationOp));
    }

    // The first use of this value (start of a linked list)
    IRUse*      firstUse = nullptr;


    // The parent of this instruction.
    IRParentInst*   parent;

    IRParentInst* getParent() { return parent; }

    // The next and previous instructions with the same parent
    IRInst*         next;
    IRInst*         prev;

    IRInst* getNextInst() { return next; }
    IRInst* getPrevInst() { return prev; }

    // The type of the result value of this instruction,
    // or `null` to indicate that the instruction has
    // no value.
    RefPtr<Type>    type;

    Type* getFullType() { return type; }

    Type* getRate();
    Type* getDataType();

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
        getOperands()[index].set(value);
    }


    //

    // Replace all uses of this value with `other`, so
    // that this value will now have no uses.
    void replaceUsesWith(IRInst* other);

    // Free a value (which needs to have been removed
    // from its parent, had its uses eliminated, etc.)
    void deallocate();

    // Clean up any non-pool resources used by this instruction
    virtual void dispose() override;

    // Insert this instruction into the same basic block
    // as `other`, right before/after it.
    void insertBefore(IRInst* other);
    void insertAfter(IRInst* other);

    // Insert as first/last child of given parent
    void insertAtStart(IRParentInst* parent);
    void insertAtEnd(IRParentInst* parent);

    // Move to the start/end of current parent
    void moveToStart();
    void moveToEnd();

    // Insert before/after the given instruction, in a specific block
    void insertBefore(IRInst* other, IRParentInst* parent);
    void insertAfter(IRInst* other, IRParentInst* parent);

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

};

// `dynamic_cast` equivalent
template<typename T>
T* as(IRInst* inst, T* /* */ = nullptr)
{
    if (inst && T::isaImpl(inst->op))
        return (T*) inst;
    return nullptr;
}

// `static_cast` equivalent, with debug validation
template<typename T>
T* cast(IRInst* inst, T* /* */ = nullptr)
{
    SLANG_ASSERT(!inst || as<T>(inst));
    return (T*)inst;
}


// A double-linked list of instruction
struct IRInstListBase
{
    IRInstListBase()
    {}

    IRInstListBase(IRInst* first, IRInst* last)
        : first(first)
        , last(last)
    {}



    IRInst* first = 0;
    IRInst* last = 0;

    IRInst* getFirst() { return first; }
    IRInst* getLast() { return last; }

    struct Iterator
    {
        IRInst* inst;

        Iterator() : inst(nullptr) {}
        Iterator(IRInst* inst) : inst(inst) {}

        void operator++()
        {
            if (inst)
            {
                inst = inst->next;
            }
        }

        IRInst* operator*()
        {
            return inst;
        }

        bool operator!=(Iterator const& i)
        {
            return inst != i.inst;
        }
    };

    Iterator begin() { return Iterator(first); }
    Iterator end() { return Iterator(last ? last->next : nullptr); }
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
    Iterator end() { return Iterator(last ? last->next : nullptr); }
};

typedef int64_t IRIntegerValue;
typedef double IRFloatingPointValue;

struct IRConstant : IRInst
{
    union
    {
        IRIntegerValue          intVal;
        IRFloatingPointValue    floatVal;

        // HACK: allows us to hash the value easily
        void*                   ptrData[2];
    } u;
};

// A instruction that ends a basic block (usually because of control flow)
struct IRTerminatorInst : IRInst
{
    static bool isaImpl(IROp op)
    {
        return (op >= kIROp_FirstTerminatorInst) && (op <= kIROp_LastTerminatorInst);
    }
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

    static bool isaImpl(IROp op) { return op == kIROp_Param; }
};

// A "parent" instruction is one that contains other instructions
// as its children. The most common case of a parent instruction
// is a basic block, but there are other cases (e.g., a function
// is in turn a parent for basic blocks).
struct IRParentInst : IRInst
{
    // The instructions stored under this parent
    IRInstListBase children;

    IRInst* getFirstChild() { return children.first; }
    IRInst* getLastChild()  { return children.last;  }
    IRInstListBase getChildren() { return children; }

    static bool isaImpl(IROp op)
    {
        return (op >= kIROp_FirstParentInst) && (op <= kIROp_LastParentInst);
    }
};

// A basic block is a parent instruction that adds the constraint
// that all the children need to be "ordinary" instructions (so
// no function declarations, or nested blocks). We also expect
// that the previous/next instruction are always a basic block.
//
struct IRBlock : IRParentInst
{
    // Linked list of the instructions contained in this block
    //
    IRInstListBase getChildren() { return children; }
    IRInst* getFirstInst() { return children.first; }
    IRInst* getLastInst() { return children.last; }

    // In a valid program, every basic block should end with
    // a "terminator" instruction.
    //
    // This function will return the terminator, if it exists,
    // or `null` if there is none.
    IRTerminatorInst* getTerminator() { return as<IRTerminatorInst>(getLastInst()); }

    // We expect that the siblings of a basic block will
    // always be other basic blocks (we don't allow
    // mixing of blocks and other instructions in the
    // same parent).
    IRBlock* getPrevBlock() { return cast<IRBlock>(getPrevInst()); }
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

        struct Iterator
        {
            Iterator(IRUse* use) : use(use) {}

            IRBlock* operator*();

            void operator++();

            bool operator!=(Iterator const& that)
            {
                return use != that.use;
            }

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

            IRUse* use;
            UInt stride;
        };

        Iterator begin() { return Iterator(begin_, stride); }
        Iterator end()   { return Iterator(end_, stride); }
    };

    PredecessorList getPredecessors();
    SuccessorList getSuccessors();

    //

    static bool isaImpl(IROp op) { return op == kIROp_Block; }
};

// For right now, we will represent the type of
// an IR function using the type of the AST
// function from which it was created.
//
// TODO: need to do this better.
typedef FuncType IRFuncType;

// A "global value" is an instruction that might have
// linkage, so that it can be declared in one module
// and then resolved to a definition in another module.
struct IRGlobalValue : IRParentInst
{
    // The mangled name, for a symbol that should have linkage,
    // or which might have multiple declarations.
    Name* mangledName = nullptr;

#if 0
    // TODO: these all belong on `IRInst`
    void insertBefore(IRGlobalValue* other);
    void insertBefore(IRGlobalValue* other, IRModule* module);
    void insertAtStart(IRModule* module);

    void insertAfter(IRGlobalValue* other);
    void insertAfter(IRGlobalValue* other, IRModule* module);
    void insertAtEnd(IRModule* module);

    void removeFromParent();

    void moveToEnd();
#endif

    static bool isaImpl(IROp op)
    {
        return (op >= kIROp_FirstGlobalValue) && (op <= kIROp_LastGlobalValue);
    }
};

/// @brief A global value that potentially holds executable code.
///
struct IRGlobalValueWithCode : IRGlobalValue
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
};

// A function is a parent to zero or more blocks of instructions.
//
// A function is itself a value, so that it can be a direct operand of
// an instruction (e.g., a call).
struct IRFunc : IRGlobalValueWithCode
{
    // The type of the IR-level function
    IRFuncType* getType() { return (IRFuncType*) type.Ptr(); }

    // If this function is generic, then we store a reference
    // to the AST-level generic that defines its parameters
    // and their constraints.
    List<RefPtr<GenericDecl>> genericDecls;
    int specializedGenericLevel = -1;

    GenericDecl* getGenericDecl()
    {
        if (specializedGenericLevel != -1)
            return genericDecls[specializedGenericLevel].Ptr();
        return nullptr;
    }

    // Convenience accessors for working with the 
    // function's type.
    Type* getResultType();
    UInt getParamCount();
    Type* getParamType(UInt index);

    // Convenience accessor for the IR parameters,
    // which are actually the parameters of the first
    // block.
    IRParam* getFirstParam();

    virtual void dispose() override
    {
        IRGlobalValueWithCode::dispose();
        genericDecls = decltype(genericDecls)();
    }
};

// The IR module itself is represented as an instruction, which
// serves at the root of the tree of all instructions in the module.
struct IRModuleInst : IRParentInst
{
    // Pointer back to the non-instruction object that represents
    // the module, so that we can get back to it in algorithms
    // that need it.
    IRModule* module;

    IRInstListBase getGlobalInsts() { return getChildren(); }
};

struct IRModule : RefObject
{
    // The compilation session in use.
    Session*    session;
    MemoryPool  memoryPool;
    List<IRObject*> irObjectsToFree; // list of ir objects to run destructor upon destruction

    IRModuleInst* moduleInst;
    IRModuleInst* getModuleInst() { return moduleInst;  }

    IRInstListBase getGlobalInsts() { return getModuleInst()->getChildren(); }

    ~IRModule()
    {
        for (auto val : irObjectsToFree)
            if (!val->isDestroyed)
                val->dispose();
        irObjectsToFree = List<IRObject*>();
    }
};

void printSlangIRAssembly(StringBuilder& builder, IRModule* module);
String getSlangIRAssembly(IRModule* module);

void dumpIR(IRModule* module);
void dumpIR(IRGlobalValue* globalVal);

String dumpIRFunc(IRFunc* func);

}


#endif
