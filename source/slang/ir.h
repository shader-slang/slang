// ir.h
#ifndef SLANG_IR_H_INCLUDED
#define SLANG_IR_H_INCLUDED

// This file defines the intermediate representation (IR) used for Slang
// shader code. This is a typed static single assignment (SSA) IR,
// similar in spirit to LLVM (but much simpler).
//

namespace Slang {

struct IRBlock;
struct IRFunc;
struct IRModule;
struct IRType;

// A value that can be referenced in the program.
struct IRValue
{
    // Type type of this value
    IRType* type;
};

// Representation of a type at the IR level.
// Such a type may not correspond to the high-level-language notion
// of a type as used by the front end.
//
// Note that types are values in the IR, so that operations
// may take type operands as easily as values.
struct IRType : IRValue
{
};

// An instruction in the program.
struct IRInst : IRValue
{
    // The basic block that contains this instruction,
    // or NULL if the instruction currently has no parent.
    IRBlock*   parentBlock;

    // The next and previous instructions in the same parent block
    IRInst*     nextInst;
    IRInst*     prevInst;
};

// A instruction that ends a basic block (usually because of control flow)
struct IRTerminatorInst : IRInst
{};

// A basic block, consisting of a sequence of instructions that can only
// be entered at the top, and can only be exited at the last instruction.
//
// Note that a block is itself a value, so that it can be a direct operand
// of an instruction (e.g., an instruction that branches to the block)
struct IRBlock : IRValue
{
    // The function that contains this block
    IRFunc* parentFunc;

    // The first and last instruction in the block (or NULL in
    // the case that the block is empty).
    //
    // Note that in a valid program, every block must end with
    // a "terminator" instruction, so these should be non-NULL,
    // and `last` should actually be an `IRTerminatorInst`.
    IRInst* first;
    IRInst* last;

    // Next and previous block in the same function
    IRBlock*    nextBlock;
    IRBlock*    prevBlock;
};

// A function parameter.
struct IRParam : IRValue
{
    // The function that declared this parameter
    IRFunc* parentFunc;

    // The next and previous parameter of the function
    IRParam*    nextParam;
    IRParam*    prevParam;

};

// A function, which consists of zero or more blocks of instructions.
//
// A function is itself a value, so that it can be a direct operand of
// an instruction (e.g., a call).
struct IRFunc : IRValue
{
    // The IR module that defines this function
    IRModule*   parentModule;

    // The unique entry block for the function is always the
    // first block in the list of blocks.
    IRBlock*    firstBlock;

    // The last block in the function.
    IRBlock*    lastBlock;

    // The parameters of the function
    IRParam*    firstParam;
    IRParam*    lastParam;

    // The next/previous function in the same IR module
    IRFunc* nextFunc;
    IRFunc* prevFunc;
};

// A module defining global values 
struct IRModule
{
};

typedef long long IRIntegerValue;
typedef double IRFloatingPointValue;


struct IRBuilder
{
    IRType* getBoolType();

    IRValue* getBoolValue(bool value);
    IRValue* getIntValue(IRType* type, IRIntegerValue value);
    IRValue* getFloatValue(IRType* type, IRFloatingPointValue value);
};

}

#endif
