// ir.h
#ifndef SLANG_IR_H_INCLUDED
#define SLANG_IR_H_INCLUDED

// This file defines the intermediate representation (IR) used for Slang
// shader code. This is a typed static single assignment (SSA) IR,
// similar in spirit to LLVM (but much simpler).
//

#include "../core/basic.h"

namespace Slang {

// TODO(tfoley): We should ditch this enumeration
// and just use the IR opcodes that represent these
// types directly. The one major complication there
// is that the order of the enum values currently
// matters, since it determines promotion rank.
// We either need to keep that restriction, or
// look up promotion rank by some other means.
//
enum class BaseType
{
    // Note(tfoley): These are ordered in terms of promotion rank, so be vareful when messing with this

    Void = 0,
    Bool,
    Int,
    UInt,
    UInt64,
    Half,
    Float,
    Double,
};


class Layout;

struct IRFunc;
struct IRInst;
struct IRModule;
struct IRParentInst;
struct IRType;

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

#if 0
enum IRPseudoOp
{
    kIRPseudoOp_Pos         = -1000,
    kIRPseudoOp_PreInc,
    kIRPseudoOp_PreDec,
    kIRPseudoOp_PostInc,
    kIRPseudoOp_PostDec,
    kIRPseudoOp_Sequence,
    kIRPseudoOp_AddAssign,
    kIRPseudoOp_SubAssign,
    kIRPseudoOp_MulAssign,
    kIRPseudoOp_DivAssign,
    kIRPseudoOp_ModAssign,
    kIRPseudoOp_AndAssign,
    kIRPseudoOp_OrAssign,
    kIRPseudoOp_XorAssign ,
    kIRPseudoOp_LshAssign,
    kIRPseudoOp_RshAssign,
    kIRPseudoOp_Assign,
    kIRPseudoOp_BitNot,
    kIRPseudoOp_And,
    kIRPseudoOp_Or,

    kIROp_Invalid = -1,
};
#endif

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
    // The value that is doing the using.
    IRInst* user;

    // The value that is being used
    IRInst* usedValue;

    // The next use of the same value
    IRUse*  nextUse;

    // A "link" back to where this use is referenced,
    // so that we can simplify updates.
    IRUse** prevLink;

    void init(IRInst* user, IRInst* usedValue);
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

typedef uint32_t IRInstID;

// In the IR, almost *everything* is an instruction,
// in order to make the representation as uniform as possible.
struct IRInst
{
    // The operation that this value represents
    IROp op;

    // A unique ID to represent the op when printing
    // (or zero to indicate that the value of this
    // op isn't special).
    IRInstID id;

    // The total number of arguments of this instruction
    // (including the type)
    uint32_t argCount;

    // The parent of this instruction.
    // This will often be a basic block, but we
    // allow instructions to nest in more general ways.
    IRParentInst*    parent;

    // The next and previous instructions in the same parent block
    IRInst*     nextInst;
    IRInst*     prevInst;

    // The first use of this value (start of a linked list)
    IRUse*      firstUse;

    // The linked list of decorations attached to this instruction
    IRDecoration* firstDecoration;

    IRDecoration* findDecorationImpl(IRDecorationOp op);

    template<typename T>
    T* findDecoration()
    {
        return (T*) findDecorationImpl(IRDecorationOp(T::kDecorationOp));
    }


    // The type of this value
    IRUse       type;

    IRType* getType() { return (IRType*) type.usedValue; }

    UInt getArgCount()
    {
        return argCount;
    }

    IRUse*      getArgs();

    IRInst* getArg(UInt index)
    {
        return getArgs()[index].usedValue;
    }
};

// This type alias exists because I waffled on the name for a bit.
// All existing uses of `IRValue` should move to `IRInst`
typedef IRInst IRValue;

typedef long long IRIntegerValue;
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

// Representation of a type at the IR level.
// Such a type may not correspond to the high-level-language notion
// of a type as used by the front end.
//
// Note that types are instructions in the IR, so that operations
// may take type operands as easily as values.
struct IRType : IRInst
{
};

// A instruction that ends a basic block (usually because of control flow)
struct IRTerminatorInst : IRInst
{};

bool isTerminatorInst(IROp op);
bool isTerminatorInst(IRInst* inst);


// A parent instruction contains a sequence of other instructions
//
struct IRParentInst : IRInst
{
    // The first and last instruction in the container (or NULL in
    // the case that the container is empty).
    //
    IRInst* firstChild;
    IRInst* lastChild;
};

// A function parameter is represented by an instruction
// in the entry block of a function.
struct IRParam : IRInst
{
    IRParam* getNextParam();
};

// A basic block is a parent instruction that adds the constraint
// that all the children need to be "ordinary" instructions (so
// no function declarations, or nested blocks). We also expect
// that the previous/next instruction are always a basic block.
//
struct IRBlock : IRParentInst
{
    // Note that in a valid program, every block must end with
    // a "terminator" instruction, so these should be non-NULL,
    // and `last` should actually be an `IRTerminatorInst`.

    IRBlock* getPrevBlock() { return (IRBlock*) prevInst; }
    IRBlock* getNextBlock() { return (IRBlock*) nextInst; }

    IRFunc* getParent() { return (IRFunc*)parent; }

    IRParam* getFirstParam();
};

struct IRFuncType;

// A function is a parent to zero or more blocks of instructions.
//
// A function is itself a value, so that it can be a direct operand of
// an instruction (e.g., a call).
struct IRFunc : IRParentInst
{
    IRFuncType* getType() { return (IRFuncType*) type.usedValue; }

    IRType* getResultType();
    UInt getParamCount();
    IRType* getParamType(UInt index);

    IRBlock* getFirstBlock() { return (IRBlock*) firstChild; }
    IRBlock* getLastBlock() { return (IRBlock*) lastChild; }

    IRParam* getFirstParam();
};

// A module is a parent to functions, global variables, types, etc.
struct IRModule : IRParentInst
{
    // The designated entry-point function, if any
    IRFunc*     entryPoint;

    // A special counter used to assign logical ids to instructions in this module.
    IRInstID    idCounter;
};

void printSlangIRAssembly(StringBuilder& builder, IRModule* module);
String getSlangIRAssembly(IRModule* module);

void dumpIR(IRModule* module);

}


#endif
