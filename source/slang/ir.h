// ir.h
#ifndef SLANG_IR_H_INCLUDED
#define SLANG_IR_H_INCLUDED

// This file defines the intermediate representation (IR) used for Slang
// shader code. This is a typed static single assignment (SSA) IR,
// similar in spirit to LLVM (but much simpler).
//

#include "../core/basic.h"

namespace Slang {

class   Decl;
class   FuncType;
class   Layout;
class   Type;
class   Session;

struct  IRFunc;
struct  IRInst;
struct  IRModule;
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
    IRInst* user;

    // The next use of the same value
    IRUse*  nextUse;

    // A "link" back to where this use is referenced,
    // so that we can simplify updates.
    IRUse** prevLink;

    void init(IRInst* user, IRValue* usedValue);
};

enum IRDecorationOp : uint16_t
{
    kIRDecorationOp_HighLevelDecl,
    kIRDecorationOp_Layout,
    kIRDecorationOp_EntryPoint,
    kIRDecorationOp_ComputeThreadGroupSize,
    kIRDecorationOp_LoopControl,
    kIRDecorationOp_MangledName,
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

// Instructions are values that can be executed,
// and which take other values as operands
struct IRInst : IRValue
{
    // The total number of arguments of this instruction.
    //
    // TODO: We shouldn't need to allocate this on
    // all instructions. Instead we should have
    // instructions that need "vararg" support to
    // allocate this field ahead of the `this`
    // pointer.
    uint32_t argCount;

    // The basic block that contains this instruction.
    IRBlock*    parentBlock;

    // The next and previous instructions in the same parent block
    IRInst*     nextInst;
    IRInst*     prevInst;

    UInt getArgCount()
    {
        return argCount;
    }

    IRUse*      getArgs();

    IRValue* getArg(UInt index)
    {
        return getArgs()[index].usedValue;
    }

    // Insert this instruction into the same basic block
    // as `other`, right before it.
    void insertBefore(IRInst* other);

    // Remove this instruction from its parent block,
    // but don't delete it, or replace uses.
    void removeFromParent();

    // Remove this instruction from its parent block,
    // and then destroy it (it had better have no uses!)
    void removeAndDeallocate();
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
    IRFunc* parentFunc;

    IRFunc* getParent() { return parentFunc; }

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

// A function is a parent to zero or more blocks of instructions.
//
// A function is itself a value, so that it can be a direct operand of
// an instruction (e.g., a call).
struct IRFunc : IRGlobalValue
{
    // The type of the IR-level function
    IRFuncType* getType() { return (IRFuncType*) type.Ptr(); }

    // The mangled name, for a function
    // that should have linkage.
    String mangledName;

    // Any generic parameters this function has
    List<RefPtr<Decl>> genericParams;

    // Convenience accessors for working with the 
    // function's type.
    Type* getResultType();
    UInt getParamCount();
    Type* getParamType(UInt index);

    // The list of basic blocks in this function
    IRBlock*    firstBlock = nullptr;
    IRBlock*    lastBlock = nullptr;

    IRBlock* getFirstBlock() { return firstBlock; }
    IRBlock* getLastBlock() { return lastBlock; }

    // Add a block to the end of this function.
    void addBlock(IRBlock* block);

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

// IR transformations

// Transform shader entry points so that they conform to GLSL rules.
void legalizeEntryPointsForGLSL(
        Session*    session,
        IRModule*   module);

}


#endif
