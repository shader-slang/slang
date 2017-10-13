// ir-insts.h
#ifndef SLANG_IR_INSTS_H_INCLUDED
#define SLANG_IR_INSTS_H_INCLUDED

// This file extends the core definitions in `ir.h`
// with a wider variety of concrete instructions,
// and a "builder" abstraction.
//
// TODO: the builder probably needs its own file.

#include "compiler.h"
#include "ir.h"
#include "syntax.h"
#include "type-layout.h"

namespace Slang {

class Decl;

// Associates an IR-level decoration with a source declaration
// in the high-level AST, that can be used to extract
// additional information that informs code emission.
struct IRHighLevelDeclDecoration : IRDecoration
{
    enum { kDecorationOp = kIRDecorationOp_HighLevelDecl };

    Decl* decl;
};

// Associates an IR-level decoration with a source layout
struct IRLayoutDecoration : IRDecoration
{
    enum { kDecorationOp = kIRDecorationOp_Layout };

    Layout* layout;
};

enum IRLoopControl
{
    kIRLoopControl_Unroll,
};

struct IRLoopControlDecoration : IRDecoration
{
    enum { kDecorationOp = kIRDecorationOp_LoopControl };

    IRLoopControl mode;
};

//

// An IR node to represent a reference to an AST-level
// declaration.
struct IRDeclRef : IRValue
{
    DeclRefBase declRef;
};

//

struct IRCall : IRInst
{
    IRUse func;
};

struct IRLoad : IRInst
{
    IRUse ptr;
};

struct IRStore : IRInst
{
    IRUse ptr;
    IRUse val;
};

struct IRFieldExtract : IRInst
{
    IRUse   base;
    IRUse   field;

    IRValue* getBase() { return base.usedValue; }
    IRValue* getField() { return field.usedValue; }
};

struct IRFieldAddress : IRInst
{
    IRUse   base;
    IRUse   field;

    IRValue* getBase() { return base.usedValue; }
    IRValue* getField() { return field.usedValue; }
};

// Terminators

struct IRReturn : IRTerminatorInst
{};

struct IRReturnVal : IRReturn
{
    IRUse val;

    IRValue* getVal() { return val.usedValue; }
};

struct IRReturnVoid : IRReturn
{};

struct IRBlock;

struct IRUnconditionalBranch : IRTerminatorInst
{
    IRUse block;

    IRBlock* getTargetBlock() { return (IRBlock*)block.usedValue; }
};

// Special cases of unconditional branch, to handle
// structured control flow:
struct IRBreak : IRUnconditionalBranch {};
struct IRContinue : IRUnconditionalBranch {};

// The start of a loop is a special control-flow
// instruction, that records relevant information
// about the loop structure:
struct IRLoop : IRUnconditionalBranch
{
    // The next block after the loop, which
    // is where we expect control flow to
    // re-converge, and also where a
    // `break` will target.
    IRUse breakBlock;

    // The block where control flow will go
    // on a `continue`.
    IRUse continueBlock;

    IRBlock* getBreakBlock() { return (IRBlock*)breakBlock.usedValue; }
    IRBlock* getContinueBlock() { return (IRBlock*)continueBlock.usedValue; }
};

struct IRConditionalBranch : IRTerminatorInst
{
    IRUse condition;
    IRUse trueBlock;
    IRUse falseBlock;

    IRValue* getCondition() { return condition.usedValue; }
    IRBlock* getTrueBlock() { return (IRBlock*)trueBlock.usedValue; }
    IRBlock* getFalseBlock() { return (IRBlock*)falseBlock.usedValue; }
};

// A conditional branch that represent the test inside a loop
struct IRLoopTest : IRConditionalBranch
{
};

// A conditional branch that represents a one-sided `if`:
//
//     if( <condition> ) { <trueBlock> }
//     <falseBlock>
struct IRIf : IRConditionalBranch
{
    IRBlock* getAfterBlock() { return getFalseBlock(); }
};

// A conditional branch that represents a two-sided `if`:
//
//     if( <condition> ) { <trueBlock> }
//     else              { <falseBlock> }
//     <afterBlock>
//
struct IRIfElse : IRConditionalBranch
{
    IRUse afterBlock;

    IRBlock* getAfterBlock() { return (IRBlock*)afterBlock.usedValue; }
};

struct IRSwizzle : IRReturn
{
    IRUse base;

    IRValue* getBase() { return base.usedValue; }
    UInt getElementCount()
    {
        return getArgCount() - 1;
    }
    IRValue* getElementIndex(UInt index)
    {
        return getArg(index + 1);
    }
};

struct IRSwizzleSet : IRReturn
{
    IRUse base;
    IRUse source;

    IRValue* getBase() { return base.usedValue; }
    IRValue* getSource() { return source.usedValue; }
    UInt getElementCount()
    {
        return getArgCount() - 2;
    }
    IRValue* getElementIndex(UInt index)
    {
        return getArg(index + 2);
    }
};

// An IR `var` instruction conceptually represents
// a stack allocation of some memory.
struct IRVar : IRInst
{
    PtrType* getType()
    {
        return (PtrType*)type.Ptr();
    }
};

struct IRGlobalVar : IRGlobalValue
{
    // TODO: should contain information
    // for use in initializing the variable
    // (e.g., a reference to a function
    // that is to be evaluated to provide
    // the initial value, or a basic block
    // that defines a DAG of constant
    // values to use as initial values...)

    PtrType* getType() { return type.As<PtrType>(); }
};



// Description of an instruction to be used for global value numbering
struct IRInstKey
{
    IRInst* inst;

    int GetHashCode();
};

bool operator==(IRInstKey const& left, IRInstKey const& right);

struct IRConstantKey
{
    IRConstant* inst;

    int GetHashCode();
};
bool operator==(IRConstantKey const& left, IRConstantKey const& right);

struct SharedIRBuilder
{
    // The parent compilation session
    Session* session;
    Session* getSession()
    {
        return session;
    }

    // The module that will own all of the IR
    IRModule*       module;

    Dictionary<IRInstKey,       IRInst*>    globalValueNumberingMap;
    Dictionary<IRConstantKey,   IRConstant*>    constantMap;
};

struct IRBuilder
{
    // Shared state for all IR builders working on the same module
    SharedIRBuilder*    shared;

    Session* getSession()
    {
        return shared->getSession();
    }

    IRModule* getModule() { return shared->module; }

    // The current function and block being inserted into
    // (or `null` if we aren't inserting).
    IRFunc*     func = nullptr;
    IRBlock*    block = nullptr;
    //
    // TODO: we eventually also want an `IRInst*` for
    // an instruction to insert before, so that we
    // can also use the builder to insert inside
    // an existing block.

    IRFunc*     getFunc() { return func; }
    IRBlock*    getBlock() { return block; }

    void addInst(IRBlock* block, IRInst* inst);
    void addInst(IRInst* inst);

    IRValue* getBoolValue(bool value);
    IRValue* getIntValue(IRType* type, IRIntegerValue value);
    IRValue* getFloatValue(IRType* type, IRFloatingPointValue value);

    IRValue* getDeclRefVal(
        DeclRefBase const&  declRef);

    IRInst* emitCallInst(
        IRType*         type,
        IRValue*        func,
        UInt            argCount,
        IRValue* const* args);

    IRInst* emitIntrinsicInst(
        IRType*         type,
        IROp            op,
        UInt            argCount,
        IRValue* const* args);

    IRInst* emitConstructorInst(
        IRType*         type,
        UInt            argCount,
        IRValue* const* args);

    IRModule* createModule();

    IRFunc* createFunc();
    IRGlobalVar* createGlobalVar(
        IRType* valueType);

    IRBlock* createBlock();
    IRBlock* emitBlock();

    IRParam* emitParam(
        IRType* type);

    IRVar* emitVar(
        IRType* type);

    IRInst* emitLoad(
        IRValue*    ptr);

    IRInst* emitStore(
        IRValue*    dstPtr,
        IRValue*    srcVal);

    IRInst* emitFieldExtract(
        IRType*         type,
        IRValue*        base,
        IRValue*        field);

    IRInst* emitFieldAddress(
        IRType*         type,
        IRValue*        basePtr,
        IRValue*        field);

    IRInst* emitElementExtract(
        IRType*     type,
        IRValue*    base,
        IRValue*    index);

    IRInst* emitElementAddress(
        IRType*     type,
        IRValue*    basePtr,
        IRValue*    index);

    IRInst* emitSwizzle(
        IRType*         type,
        IRValue*        base,
        UInt            elementCount,
        IRValue* const* elementIndices);

    IRInst* emitSwizzle(
        IRType*         type,
        IRValue*        base,
        UInt            elementCount,
        UInt const*     elementIndices);

    IRInst* emitSwizzleSet(
        IRType*         type,
        IRValue*        base,
        IRValue*        source,
        UInt            elementCount,
        IRValue* const* elementIndices);

    IRInst* emitSwizzleSet(
        IRType*         type,
        IRValue*        base,
        IRValue*        source,
        UInt            elementCount,
        UInt const*     elementIndices);

    IRInst* emitReturn(
        IRValue*    val);

    IRInst* emitReturn();

    IRInst* emitBranch(
        IRBlock*    block);

    IRInst* emitBreak(
        IRBlock*    target);

    IRInst* emitContinue(
        IRBlock*    target);

    IRInst* emitLoop(
        IRBlock*    target,
        IRBlock*    breakBlock,
        IRBlock*    continueBlock);

    IRInst* emitBranch(
        IRValue*    val,
        IRBlock*    trueBlock,
        IRBlock*    falseBlock);

    IRInst* emitIf(
        IRValue*    val,
        IRBlock*    trueBlock,
        IRBlock*    afterBlock);

    IRInst* emitIfElse(
        IRValue*    val,
        IRBlock*    trueBlock,
        IRBlock*    falseBlock,
        IRBlock*    afterBlock);

    IRInst* emitLoopTest(
        IRValue*    val,
        IRBlock*    bodyBlock,
        IRBlock*    breakBlock);

    IRDecoration* addDecorationImpl(
        IRValue*        value,
        UInt            decorationSize,
        IRDecorationOp  op);

    template<typename T>
    T* addDecoration(IRValue* value, IRDecorationOp op)
    {
        return (T*) addDecorationImpl(value, sizeof(T), op);
    }

    template<typename T>
    T* addDecoration(IRValue* value)
    {
        return (T*) addDecorationImpl(value, sizeof(T), IRDecorationOp(T::kDecorationOp));
    }

    IRHighLevelDeclDecoration* addHighLevelDeclDecoration(IRValue* value, Decl* decl);
    IRLayoutDecoration* addLayoutDecoration(IRValue* value, Layout* layout);
};

// Generate a clone of an IR module that is specialized for
// a particular entry point, target, etc.

IRModule* specializeIRForEntryPoint(
    EntryPointRequest*  entryPointRequest,
    ProgramLayout*      programLayout,
    CodeGenTarget       target);


}

#endif
