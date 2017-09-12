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

// Identifies a function as an entry point for some stage
struct IREntryPointDecoration : IRDecoration
{
    enum { kDecorationOp = kIRDecorationOp_EntryPoint };

    Profile profile;
};

// Associates a compute-shader entry point function
// with a thread-group size.
struct IRComputeThreadGroupSizeDecoration : IRDecoration
{
    enum { kDecorationOp = kIRDecorationOp_ComputeThreadGroupSize };

    UInt sizeAlongAxis[3];
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

// Types

struct IRVectorType : IRType
{
    IRUse   elementType;
    IRUse   elementCount;

    IRType* getElementType() { return (IRType*) elementType.usedValue; }
    IRInst* getElementCount() { return elementCount.usedValue; }
};

struct IRMatrixType : IRType
{
    IRUse   elementType;
    IRUse   rowCount;
    IRUse   columnCount;

    IRType* getElementType() { return (IRType*) elementType.usedValue; }
    IRInst* getRowCount() { return rowCount.usedValue; }
    IRInst* getColumnCount() { return columnCount.usedValue; }
};

struct IRArrayType : IRType
{
    IRUse   elementType;
    IRUse   elementCount;

    IRType* getElementType() { return (IRType*) elementType.usedValue; }
    IRInst* getElementCount() { return elementCount.usedValue; }
};



struct IRFuncType : IRType
{
    IRUse resultType;
    // parameter tyeps are varargs...

    IRType* getResultType() { return (IRType*) resultType.usedValue; }
    UInt getParamCount()
    {
        return getArgCount() - 2;
    }
    IRType* getParamType(UInt index)
    {
        return (IRType*) getArg(2 + index);
    }
};

// Address spaces for IR pointers
enum IRAddressSpace : UInt
{
    // A default address space for things like local variables
    kIRAddressSpace_Default,

    // Address space for HLSL `groupshared` allocations
    kIRAddressSpace_GroupShared,
};

struct IRPtrType : IRType
{
    IRUse valueType;
    IRUse addressSpace;

    IRType* getValueType() { return (IRType*) valueType.usedValue; }

    IRAddressSpace getAddressSpace()
    {
        return IRAddressSpace(
            ((IRConstant*)addressSpace.usedValue)->u.intVal);
    }
};

struct IRTextureType : IRType
{
    IRUse flavor;
    IRUse elementType;

    IRIntegerValue getFlavor() { return ((IRConstant*) flavor.usedValue)->u.intVal; }
    IRType* getElementType() { return (IRType*) elementType.usedValue; }
};

struct IRBufferType : IRType
{
    IRUse elementType;
    IRType* getElementType() { return (IRType*) elementType.usedValue; }
};


struct IRUniformBufferType : IRType
{
    IRUse elementType;
    IRType* getElementType() { return (IRType*) elementType.usedValue; }
};

struct IRConstantBufferType : IRUniformBufferType {};
struct IRTextureBufferType : IRUniformBufferType {};

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

struct IRStructField;
struct IRFieldExtract : IRInst
{
    IRUse   base;
    IRUse   field;

    IRInst* getBase() { return base.usedValue; }
    IRStructField* getField() { return (IRStructField*) field.usedValue; }
};

struct IRFieldAddress : IRInst
{
    IRUse   base;
    IRUse   field;

    IRInst* getBase() { return base.usedValue; }
    IRStructField* getField() { return (IRStructField*) field.usedValue; }
};

// Terminators

struct IRReturn : IRTerminatorInst
{};

struct IRReturnVal : IRReturn
{
    IRUse val;

    IRInst* getVal() { return val.usedValue; }
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

    IRInst* getCondition() { return condition.usedValue; }
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

    IRInst* getBase() { return base.usedValue; }
    UInt getElementCount()
    {
        return getArgCount() - 2;
    }
    IRInst* getElementIndex(UInt index)
    {
        return getArg(index + 2);
    }
};

struct IRSwizzleSet : IRReturn
{
    IRUse base;
    IRUse source;

    IRInst* getBase() { return base.usedValue; }
    IRInst* getSource() { return source.usedValue; }
    UInt getElementCount()
    {
        return getArgCount() - 3;
    }
    IRInst* getElementIndex(UInt index)
    {
        return getArg(index + 3);
    }
};

// "Parent" Instructions (Declarations)

struct IRStructField : IRInst
{
    IRType* getFieldType() { return (IRType*) type.usedValue; }

    IRStructField* getNextField() { return (IRStructField*) nextInst; }
};

struct IRStructDecl : IRParentInst
{
    IRStructField* getFirstField() { return (IRStructField*) firstChild; }
    IRStructField* getLastField() { return (IRStructField*) lastChild; }
};


struct IRVar : IRInst
{
    IRPtrType* getType()
    {
        return (IRPtrType*)type.usedValue;
    }
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
    // The module that will own all of the IR
    IRModule*       module;

    Dictionary<IRInstKey,       IRInst*>    globalValueNumberingMap;
    Dictionary<IRConstantKey,   IRConstant*>    constantMap;
};

struct IRBuilder
{
    // Shared state for all IR builders working on the same module
    SharedIRBuilder*    shared;

    IRModule* getModule() { return shared->module; }

    // The parent instruction to add children to.
    IRParentInst*   parentInst;

    void addInst(IRParentInst* parent, IRInst* inst);
    void addInst(IRInst* inst);

    IRType* getBaseType(BaseType flavor);
    IRType* getBoolType();
    IRType* getVectorType(IRType* elementType, IRValue* elementCount);
    IRType* getMatrixType(
        IRType* elementType,
        IRValue* rowCount,
        IRValue* columnCount);
    IRType* getArrayType(IRType* elementType, IRValue* elementCount);
    IRType* getArrayType(IRType* elementType);

    IRType* getTypeType();
    IRType* getVoidType();
    IRType* getBlockType();

    IRType* getIntrinsicType(
        IROp op,
        UInt argCount,
        IRValue* const* args);

    IRStructDecl* createStructType();
    IRStructField* createStructField(IRType* fieldType);

    IRType* getFuncType(
        UInt            paramCount,
        IRType* const*  paramTypes,
        IRType*         resultType);

    IRType* getPtrType(
        IRType*         valueType,
        IRAddressSpace  addressSpace);

    IRType* getPtrType(
        IRType* valueType);

    IRValue* getBoolValue(bool value);
    IRValue* getIntValue(IRType* type, IRIntegerValue value);
    IRValue* getFloatValue(IRType* type, IRFloatingPointValue value);

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

    IRBlock* createBlock();
    IRBlock* emitBlock();

    IRParam* emitParam(
        IRType* type);

    IRVar* emitVar(
        IRType*         type,
        IRAddressSpace  addressSpace);

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
        IRStructField*  field);

    IRInst* emitFieldAddress(
        IRType*         type,
        IRValue*        basePtr,
        IRStructField*  field);

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
        IRInst*         inst,
        UInt            decorationSize,
        IRDecorationOp  op);

    template<typename T>
    T* addDecoration(IRInst* inst, IRDecorationOp op)
    {
        return (T*) addDecorationImpl(inst, sizeof(T), op);
    }

    template<typename T>
    T* addDecoration(IRInst* inst)
    {
        return (T*) addDecorationImpl(inst, sizeof(T), IRDecorationOp(T::kDecorationOp));
    }

    IRHighLevelDeclDecoration* addHighLevelDeclDecoration(IRInst* inst, Decl* decl);
    IRLayoutDecoration* addLayoutDecoration(IRInst* inst, Layout* layout);
};

}

#endif
