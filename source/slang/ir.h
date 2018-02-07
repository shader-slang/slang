// ir.h
#ifndef SLANG_IR_H_INCLUDED
#define SLANG_IR_H_INCLUDED

// This file defines the intermediate representation (IR) used for Slang
// shader code. This is a typed static single assignment (SSA) IR,
// similar in spirit to LLVM (but much simpler).
//

#include "../core/basic.h"

#include "source-loc.h"

namespace Slang {

class   Decl;
class   GenericDecl;
class   FuncType;
class   Layout;
class   Type;
class   Session;

struct  IRFunc;
struct  IRGlobalValueWithCode;
struct  IRInst;
struct  IRModule;
struct  IRUser;
struct  IRValue;

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
    // The value that is being used
    IRValue* usedValue;

    // The value that is doing the using.
    IRUser* user;

    // The next use of the same value
    IRUse*  nextUse;

    // A "link" back to where this use is referenced,
    // so that we can simplify updates.
    IRUse** prevLink;

    void init(IRUser* user, IRValue* usedValue);
    void set(IRValue* usedValue);
    void clear();
};

enum IRDecorationOp : uint16_t
{
    kIRDecorationOp_HighLevelDecl,
    kIRDecorationOp_Layout,
    kIRDecorationOp_LoopControl,
    kIRDecorationOp_Target,
    kIRDecorationOp_TargetIntrinsic,
};

// A "decoration" that gets applied to an instruction.
// These usually don't affect semantics, but are useful
// for preserving high-level source information.
struct IRDecoration
{
    // Next decoration attached to the same instruction
    IRDecoration* next;

    IRDecorationOp op;
};

// Use AST-level types directly to represent the
// types of IR instructions/values
typedef Type IRType;

struct IRBlock;

// Base class for values in the IR
struct IRValue
{
    // The operation that this value represents
    IROp op;

    // The type of the result value of this instruction,
    // or `null` to indicate that the instruction has
    // no value.
    RefPtr<Type>    type;

    Type* getType() { return type; }

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


    // Replace all uses of this value with `other`, so
    // that this value will now have no uses.
    void replaceUsesWith(IRValue* other);

    // Free a value (which needs to have been removed
    // from its parent, had its uses eliminated, etc.)
    void deallocate();
};

// Values that are contained in a doubly-linked
// list inside of some parent.
//
// TODO: consider merging this into `IRValue` so
// that *all* values have a parent.
struct IRChildValue : IRValue
{
    // The parent of this value.
    IRValue*        parent;

    // The next and previous values in the same
    // list on teh same parent.
    IRChildValue*   next;
    IRChildValue*   prev;
};

// Helper for storing linked lists of child values.
struct IRValueListBase
{
    IRChildValue* first = 0;
    IRChildValue* last = 0;

protected:
    void addImpl(IRValue* parent, IRChildValue* val);
};
template<typename T>
struct IRValueList : IRValueListBase
{
    T* getFirst() { return (T*)first; }
    T* getLast() { return (T*)last; }

    void add(IRValue* parent, T* val)
    {
        addImpl(parent, val);
    }

    struct Iterator
    {
        T* val;

        Iterator() : val(0) {}
        Iterator(T* val) : val(val) {}

        void operator++()
        {
            if (val)
            {
                val = (T*)val->next;
            }
        }

        T* operator*()
        {
            return val;
        }

        bool operator!=(Iterator const& i)
        {
            return val != i.val;
        }
    };

    Iterator begin() { return Iterator(getFirst()); }
    Iterator end() { return Iterator(nullptr); }
};

// Values that can use other values. These always
// have their operands "tail allocated" after
// the fields of this type, so derived types must
// either:
//
// - Add no new fields, or
// - Add only fields that represent the `IRUse` operands
// - Add a fixed number of `IRUse` operand fields and
//   then any additional data after them.
//
struct IRUser : IRChildValue
{
    // The total number of arguments of this instruction.
    //
    // TODO: We shouldn't need to allocate this on
    // all instructions. Instead we should have
    // instructions that need "vararg" support to
    // allocate this field ahead of the `this`
    // pointer.
    uint32_t argCount;

    UInt getArgCount()
    {
        return argCount;
    }

    IRUse*      getArgs();

    IRValue* getArg(UInt index)
    {
        return getArgs()[index].usedValue;
    }

    void setArg(UInt index, IRValue* value)
    {
        getArgs()[index].set(value);
    }
};

// Instructions are values that are children of a basic block,
// and can actually be executed.
struct IRInst : IRUser
{
    IRBlock* getParentBlock() { return (IRBlock*)parent; }

    IRInst* getPrevInst() { return (IRInst*)prev; }
    IRInst* getNextInst() { return (IRInst*)next; }


    // Insert this instruction into the same basic block
    // as `other`, right before it.
    void insertBefore(IRInst* other);

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

typedef int64_t IRIntegerValue;
typedef double IRFloatingPointValue;

struct IRConstant : IRValue
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
{};

bool isTerminatorInst(IROp op);
bool isTerminatorInst(IRInst* inst);

// A function parameter is owned by a basic block, and represents
// either an incoming function parameter (in the entry block), or
// a value that flows from one SSA block to another (in a non-entry
// block).
//
// In each case, the basic idea is that a block is a "label with
// arguments."
struct IRParam : IRValue
{
    IRParam*    nextParam;
    IRParam*    prevParam;

    IRParam* getNextParam() { return nextParam; }
    IRParam* getPrevParam() { return prevParam; }
};

// A basic block is a parent instruction that adds the constraint
// that all the children need to be "ordinary" instructions (so
// no function declarations, or nested blocks). We also expect
// that the previous/next instruction are always a basic block.
//
struct IRBlock : IRValue
{
    // Linked list of the instructions contained in this block
    //
    // Note that in a valid program, every block must end with
    // a "terminator" instruction, so these should be non-NULL,
    // and `lastInst` should actually be an `IRTerminatorInst`.
    IRInst* firstInst;
    IRInst* lastInst;

    IRInst* getFirstInst() { return firstInst; }
    IRInst* getLastInst() { return lastInst; }

    // Links for the list of basic blocks in the parent function
    IRBlock* prevBlock;
    IRBlock* nextBlock;

    IRBlock* getPrevBlock() { return prevBlock; }
    IRBlock* getNextBlock() { return nextBlock; }

    // Linked list of parameters of this block
    IRParam* firstParam;
    IRParam* lastParam;

    IRParam* getFirstParam() { return firstParam; }
    IRParam* getLastParam() { return lastParam; }
    void addParam(IRParam* param);

    // The parent function that contains this block
    IRGlobalValueWithCode* parentFunc;

    IRGlobalValueWithCode* getParent() { return parentFunc; }

    void insertAfter(IRBlock* other);
    void insertAfter(IRBlock* other, IRGlobalValueWithCode* func);

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
};

// For right now, we will represent the type of
// an IR function using the type of the AST
// function from which it was created.
//
// TODO: need to do this better.
typedef FuncType IRFuncType;

struct IRGlobalValue : IRValue
{
    IRModule*   parentModule;

    // The mangled name, for a symbol that should have linkage,
    // or which might have multiple declarations.
    String mangledName;


    IRGlobalValue*  nextGlobalValue;
    IRGlobalValue*  prevGlobalValue;

    IRGlobalValue*  getNextValue() { return nextGlobalValue; }
    IRGlobalValue*  getPrevValue() { return prevGlobalValue; }

    void insertBefore(IRGlobalValue* other);
    void insertBefore(IRGlobalValue* other, IRModule* module);
    void insertAtStart(IRModule* module);

    void insertAfter(IRGlobalValue* other);
    void insertAfter(IRGlobalValue* other, IRModule* module);
    void insertAtEnd(IRModule* module);

    void removeFromParent();

    void moveToEnd();
};

/// @brief A global value that potentially holds executable code.
///
struct IRGlobalValueWithCode : IRGlobalValue
{
    // The list of basic blocks in this function
    IRBlock*    firstBlock = nullptr;
    IRBlock*    lastBlock = nullptr;

    IRBlock* getFirstBlock() { return firstBlock; }
    IRBlock* getLastBlock() { return lastBlock; }

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
};

// A module is a parent to functions, global variables, types, etc.
struct IRModule : RefObject
{
    // The compilation session in use.
    Session*    session;

    // A list of all the functions and other
    // global values declared in this module.
    IRGlobalValue*  firstGlobalValue = nullptr;
    IRGlobalValue*  lastGlobalValue = nullptr;

    IRGlobalValue*  getFirstGlobalValue() { return firstGlobalValue; }
    IRGlobalValue*  getlastGlobalValue() { return lastGlobalValue; }
};

void printSlangIRAssembly(StringBuilder& builder, IRModule* module);
String getSlangIRAssembly(IRModule* module);

void dumpIR(IRModule* module);
String dumpIRFunc(IRFunc* func);

}


#endif
