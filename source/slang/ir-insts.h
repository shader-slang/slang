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

    RefPtr<Layout>  layout;
    virtual void dispose() override
    {
        IRDecoration::dispose();
        layout = nullptr;
    }
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


struct IRTargetSpecificDecoration : IRDecoration
{
    // TODO: have a more structured representation of target specifiers
    String targetName;
    virtual void dispose()override
    {
        IRDecoration::dispose();
        targetName = String();
    }
};

struct IRTargetDecoration : IRTargetSpecificDecoration
{
    enum { kDecorationOp = kIRDecorationOp_Target };
};

struct IRTargetIntrinsicDecoration : IRTargetSpecificDecoration
{
    enum { kDecorationOp = kIRDecorationOp_TargetIntrinsic };

    String definition;
    virtual void dispose()override
    {
        IRTargetSpecificDecoration::dispose();
        definition = String();
    }
};

//

// An IR node to represent a reference to an AST-level
// declaration.
struct IRDeclRef : IRInst
{
    DeclRef<Decl> declRef;
    virtual void dispose() override
    {
        IRInst::dispose();
        declRef = decltype(declRef)();
    }
};

// An instruction that specializes another IR value
// (representing a generic) to a particular set of
// generic arguments (encoded via an `IRDeclRef`)
//
struct IRSpecialize : IRInst
{
    IRUse genericVal;
    IRUse specDeclRefVal;
};

// An instruction that looks up the implementation
// of an interface operation identified by `requirementDeclRef`
// in the witness table `witnessTable` which should
// hold the conformance information for a specific type.
struct IRLookupWitnessMethod : IRInst
{
    IRUse witnessTable;
    IRUse requirementDeclRef;
};

struct IRLookupWitnessTable : IRInst
{
    IRUse sourceType;
    IRUse interfaceType;
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

    IRInst* getBase() { return base.get(); }
    IRInst* getField() { return field.get(); }
};

struct IRFieldAddress : IRInst
{
    IRUse   base;
    IRUse   field;

    IRInst* getBase() { return base.get(); }
    IRInst* getField() { return field.get(); }
};

// Terminators

struct IRReturn : IRTerminatorInst
{};

struct IRReturnVal : IRReturn
{
    IRUse val;

    IRInst* getVal() { return val.get(); }
};

struct IRReturnVoid : IRReturn
{};

struct IRDiscard : IRTerminatorInst
{};

// Signals that this point in the code should be unreachable.
// We can/should emit a dataflow error if we can ever determine
// that a block ending in one of these can actually be
// executed.
struct IRUnreachable : IRTerminatorInst
{};

struct IRBlock;

struct IRUnconditionalBranch : IRTerminatorInst
{
    IRUse block;

    IRBlock* getTargetBlock() { return (IRBlock*)block.get(); }
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

    IRBlock* getBreakBlock() { return (IRBlock*)breakBlock.get(); }
    IRBlock* getContinueBlock() { return (IRBlock*)continueBlock.get(); }
};

struct IRConditionalBranch : IRTerminatorInst
{
    IRUse condition;
    IRUse trueBlock;
    IRUse falseBlock;

    IRInst* getCondition() { return condition.get(); }
    IRBlock* getTrueBlock() { return (IRBlock*)trueBlock.get(); }
    IRBlock* getFalseBlock() { return (IRBlock*)falseBlock.get(); }
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

    IRBlock* getAfterBlock() { return (IRBlock*)afterBlock.get(); }
};

// A multi-way branch that represents a source-level `switch`
struct IRSwitch : IRTerminatorInst
{
    IRUse condition;
    IRUse breakLabel;
    IRUse defaultLabel;

    IRInst* getCondition() { return condition.get(); }
    IRBlock* getBreakLabel() { return (IRBlock*) breakLabel.get(); }
    IRBlock* getDefaultLabel() { return (IRBlock*) defaultLabel.get(); }

    // remaining args are: caseVal, caseLabel, ...

    UInt getCaseCount() { return (getOperandCount() - 3) / 2; }
    IRInst* getCaseValue(UInt index) { return            getOperand(3 + index*2 + 0); }
    IRBlock* getCaseLabel(UInt index) { return (IRBlock*) getOperand(3 + index*2 + 1); }
};

struct IRSwizzle : IRReturn
{
    IRUse base;

    IRInst* getBase() { return base.get(); }
    UInt getElementCount()
    {
        return getOperandCount() - 1;
    }
    IRInst* getElementIndex(UInt index)
    {
        return getOperand(index + 1);
    }
};

struct IRSwizzleSet : IRReturn
{
    IRUse base;
    IRUse source;

    IRInst* getBase() { return base.get(); }
    IRInst* getSource() { return source.get(); }
    UInt getElementCount()
    {
        return getOperandCount() - 2;
    }
    IRInst* getElementIndex(UInt index)
    {
        return getOperand(index + 2);
    }
};

// An IR `var` instruction conceptually represents
// a stack allocation of some memory.
struct IRVar : IRInst
{
    PtrType* getDataType()
    {
        return (PtrType*) IRInst::getDataType();
    }

    static bool isaImpl(IROp op) { return op == kIROp_Var; }
};

/// @brief A global variable.
///
/// Represents a global variable in the IR.
/// If the variable has an initializer, then
/// it is represented by the code in the basic
/// blocks nested inside this value.
struct IRGlobalVar : IRGlobalValueWithCode
{
    PtrType* getDataType()
    {
        return (PtrType*) IRInst::getDataType();
    }
};

/// @brief A global constant.
///
/// Represents a global-scope constant value in the IR.
/// The initializer for the constant is represented by
/// the code in the basic block(s) nested in this value.
struct IRGlobalConstant : IRGlobalValueWithCode
{
};

// An entry in a witness table (see below)
struct IRWitnessTableEntry : IRInst
{
    // The AST-level requirement
    IRUse requirementKey;

    // The IR-level value that satisfies the requirement
    IRUse satisfyingVal;
};

// A witness table is a global value that stores
// information about how a type conforms to some
// interface. It basically takes the form of a
// map from the required members of the interface
// to the IR values that satisfy those requirements.
struct IRWitnessTable : IRGlobalValue
{
    IRInstList<IRWitnessTableEntry> getEntries()
    {
        return IRInstList<IRWitnessTableEntry>(getChildren());
    }

    RefPtr<GenericDecl> genericDecl;
    DeclRef<Decl> subTypeDeclRef, supTypeDeclRef;

    virtual void dispose() override
    {
        IRGlobalValue::dispose();
        genericDecl = decltype(genericDecl)();
        subTypeDeclRef = decltype(subTypeDeclRef)();
        supTypeDeclRef = decltype(supTypeDeclRef)();
    }
};

// An instruction that yields an undefined value.
//
// Note that we make this an instruction rather than a value,
// so that we will be able to identify a variable that is
// used when undefined.
struct IRUndefined : IRInst
{
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
    Dictionary<Name*, IRWitnessTable*> witnessTableMap;
};

struct IRBuilderSourceLocRAII;

struct IRBuilder
{
    // Shared state for all IR builders working on the same module
    SharedIRBuilder*    sharedBuilder;

    Session* getSession()
    {
        return sharedBuilder->getSession();
    }

    IRModule* getModule() { return sharedBuilder->module; }

    // The current function and block being inserted into
    // (or `null` if we aren't inserting).
    IRGlobalValueWithCode*  curFunc = nullptr;
    IRBlock*                curBlock = nullptr;
    //
    // An instruction in the current block that we should insert before
    IRInst*     insertBeforeInst = nullptr;

    IRGlobalValueWithCode*  getFunc() { return curFunc; }
    IRBlock*                getBlock() { return curBlock; }

    IRBuilderSourceLocRAII* sourceLocInfo = nullptr;

    void addInst(IRInst* inst);

    IRInst* getBoolValue(bool value);
    IRInst* getIntValue(IRType* type, IRIntegerValue value);
    IRInst* getFloatValue(IRType* type, IRFloatingPointValue value);

    IRInst* getDeclRefVal(
        DeclRefBase const&  declRef);
    IRInst* getTypeVal(IRType* type); // create an IR value that represents a type
    IRInst* emitSpecializeInst(
        IRType*     type,
        IRInst*    genericVal,
        IRInst*    specDeclRef);

    IRInst* emitSpecializeInst(
        IRType*         type,
        IRInst*        genericVal,
        DeclRef<Decl>   specDeclRef);

    IRInst* emitLookupInterfaceMethodInst(
        IRType*     type,
        IRInst*    witnessTableVal,
        IRInst*    interfaceMethodVal);

    IRInst* emitLookupInterfaceMethodInst(
        IRType*         type,
        DeclRef<Decl>   witnessTableDeclRef,
        DeclRef<Decl>   interfaceMethodDeclRef);

    IRInst* emitLookupInterfaceMethodInst(
        IRType*         type,
        IRInst*   witnessTableVal,
        DeclRef<Decl>   interfaceMethodDeclRef);

    IRInst* emitFindWitnessTable(
        DeclRef<Decl> baseTypeDeclRef,
        IRType* interfaceType);

    IRInst* emitCallInst(
        IRType*         type,
        IRInst*        func,
        UInt            argCount,
        IRInst* const* args);

    IRInst* emitIntrinsicInst(
        IRType*         type,
        IROp            op,
        UInt            argCount,
        IRInst* const* args);

    IRInst* emitConstructorInst(
        IRType*         type,
        UInt            argCount,
        IRInst* const* args);

    IRInst* emitMakeVector(
        IRType*         type,
        UInt            argCount,
        IRInst* const* args);

    IRInst* emitMakeArray(
        IRType*         type,
        UInt            argCount,
        IRInst* const* args);

    IRInst* emitMakeStruct(
        IRType*         type,
        UInt            argCount,
        IRInst* const* args);

    IRUndefined* emitUndefined(IRType* type);



    IRModule* createModule();
    
    IRFunc* createFunc();
    IRGlobalVar* createGlobalVar(
        IRType* valueType);
    IRGlobalConstant* createGlobalConstant(
        IRType* valueType);
    IRWitnessTable* createWitnessTable();
    IRWitnessTableEntry* createWitnessTableEntry(
        IRWitnessTable* witnessTable,
        IRInst*        requirementKey,
        IRInst*        satisfyingVal);
    IRWitnessTable* lookupWitnessTable(Name* mangledName);
    void registerWitnessTable(IRWitnessTable* table);
    IRBlock* createBlock();
    IRBlock* emitBlock();

    IRParam* createParam(
        IRType* type);
    IRParam* emitParam(
        IRType* type);

    IRVar* emitVar(
        IRType* type);

    IRInst* emitLoad(
        IRInst*    ptr);

    IRInst* emitStore(
        IRInst*    dstPtr,
        IRInst*    srcVal);

    IRInst* emitFieldExtract(
        IRType*         type,
        IRInst*        base,
        IRInst*        field);

    IRInst* emitFieldAddress(
        IRType*         type,
        IRInst*        basePtr,
        IRInst*        field);

    IRInst* emitElementExtract(
        IRType*     type,
        IRInst*    base,
        IRInst*    index);

    IRInst* emitElementAddress(
        IRType*     type,
        IRInst*    basePtr,
        IRInst*    index);

    IRInst* emitSwizzle(
        IRType*         type,
        IRInst*        base,
        UInt            elementCount,
        IRInst* const* elementIndices);

    IRInst* emitSwizzle(
        IRType*         type,
        IRInst*        base,
        UInt            elementCount,
        UInt const*     elementIndices);

    IRInst* emitSwizzleSet(
        IRType*         type,
        IRInst*        base,
        IRInst*        source,
        UInt            elementCount,
        IRInst* const* elementIndices);

    IRInst* emitSwizzleSet(
        IRType*         type,
        IRInst*        base,
        IRInst*        source,
        UInt            elementCount,
        UInt const*     elementIndices);

    IRInst* emitReturn(
        IRInst*    val);

    IRInst* emitReturn();

    IRInst* emitDiscard();

    IRInst* emitUnreachable();

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
        IRInst*    val,
        IRBlock*    trueBlock,
        IRBlock*    falseBlock);

    IRInst* emitIf(
        IRInst*    val,
        IRBlock*    trueBlock,
        IRBlock*    afterBlock);

    IRInst* emitIfElse(
        IRInst*    val,
        IRBlock*    trueBlock,
        IRBlock*    falseBlock,
        IRBlock*    afterBlock);

    IRInst* emitLoopTest(
        IRInst*    val,
        IRBlock*    bodyBlock,
        IRBlock*    breakBlock);

    IRInst* emitSwitch(
        IRInst*        val,
        IRBlock*        breakLabel,
        IRBlock*        defaultLabel,
        UInt            caseArgCount,
        IRInst* const* caseArgs);

    template<typename T>
    T* addDecoration(IRInst* value, IRDecorationOp op)
    {
        assert(getModule());
        auto decorationSize = sizeof(T);
        auto decoration = (T*)getModule()->memoryPool.allocZero(decorationSize);
        new(decoration)T();
       
        decoration->op = op;

        decoration->next = value->firstDecoration;
        value->firstDecoration = decoration;
        getModule()->irObjectsToFree.Add(decoration);
        return decoration;
    }

    template<typename T>
    T* addDecoration(IRInst* value)
    {
        return addDecoration<T>(value, IRDecorationOp(T::kDecorationOp));
    }

    IRHighLevelDeclDecoration* addHighLevelDeclDecoration(IRInst* value, Decl* decl);
    IRLayoutDecoration* addLayoutDecoration(IRInst* value, Layout* layout);
};

// Helper to establish the source location that will be used
// by an IRBuilder.
struct IRBuilderSourceLocRAII
{
    IRBuilder*  builder;
    SourceLoc   sourceLoc;
    IRBuilderSourceLocRAII* next;

    IRBuilderSourceLocRAII(
        IRBuilder*  builder,
        SourceLoc   sourceLoc)
        : builder(builder)
        , sourceLoc(sourceLoc)
        , next(nullptr)
    {
        next = builder->sourceLocInfo;
        builder->sourceLocInfo = this;
    }

    ~IRBuilderSourceLocRAII()
    {
        assert(builder->sourceLocInfo == this);
        builder->sourceLocInfo = next;
    }
};


//

// Interface to IR specialization for use when cloning target-specific
// IR as part of compiling an entry point.
//
// TODO: we really need to move all of this logic to its own files.

// `IRSpecializationState` is used as an opaque type to wrap up all
// the data needed to perform IR specialization, without exposing
// implementation details.
struct IRSpecializationState;
IRSpecializationState* createIRSpecializationState(
    EntryPointRequest*  entryPointRequest,
    ProgramLayout*      programLayout,
    CodeGenTarget       target,
    TargetRequest*      targetReq);
void destroyIRSpecializationState(IRSpecializationState* state);
IRModule* getIRModule(IRSpecializationState* state);

IRGlobalValue* getSpecializedGlobalValueForDeclRef(
    IRSpecializationState*  state,
    DeclRef<Decl> const&    declRef);

struct ExtensionUsageTracker;

// Clone the IR values reachable from the given entry point
// into the IR module assocaited with the specialization state.
// When multiple definitions of a symbol are found, the one
// that is best specialized for the given `targetReq` will be
// used.
void specializeIRForEntryPoint(
    IRSpecializationState*  state,
    EntryPointRequest*  entryPointRequest,
    ExtensionUsageTracker*  extensionUsageTracker);

// Find suitable uses of the `specialize` instruction that
// can be replaced with references to specialized functions.
void specializeGenerics(
    IRModule*   module,
    CodeGenTarget target);

//

void markConstExpr(
    Session*    session,
    IRInst*     irValue);

//

}

#endif
