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

struct IRDecoration : IRInst
{
    IR_PARENT_ISA(Decoration)

    IRDecoration* getNextDecoration()
    {
        return as<IRDecoration>(getNextInst());
    }
};

// Associates an IR-level decoration with a source declaration
// in the high-level AST, that can be used to extract
// additional information that informs code emission.
struct IRHighLevelDeclDecoration : IRDecoration
{
    enum { kOp = kIROp_HighLevelDeclDecoration };
    IR_LEAF_ISA(HighLevelDeclDecoration)

    IRPtrLit* getDeclOperand() { return cast<IRPtrLit>(getOperand(0)); }
    Decl* getDecl() { return (Decl*) getDeclOperand()->getValue(); }
};

// Associates an IR-level decoration with a source layout
struct IRLayoutDecoration : IRDecoration
{
    enum { kOp = kIROp_LayoutDecoration };
    IR_LEAF_ISA(LayoutDecoration)

    IRPtrLit* getLayoutOperand() { return cast<IRPtrLit>(getOperand(0)); }
    Layout* getLayout() { return (Layout*) getLayoutOperand()->getValue(); }
};

enum IRLoopControl
{
    kIRLoopControl_Unroll,
};

struct IRLoopControlDecoration : IRDecoration
{
    enum { kOp = kIROp_LoopControlDecoration };
    IR_LEAF_ISA(LoopControlDecoration)

    IRConstant* getModeOperand() { return cast<IRConstant>(getOperand(0)); }

    IRLoopControl getMode()
    {
        return IRLoopControl(getModeOperand()->value.intVal);
    }
};


struct IRTargetSpecificDecoration : IRDecoration
{
    IR_PARENT_ISA(TargetSpecificDecoration)

    IRStringLit* getTargetNameOperand() { return cast<IRStringLit>(getOperand(0)); }

    UnownedStringSlice getTargetName()
    {
        return getTargetNameOperand()->getStringSlice();
    }
};

struct IRTargetDecoration : IRTargetSpecificDecoration
{
    enum { kOp = kIROp_TargetDecoration };
    IR_LEAF_ISA(TargetDecoration)
};

struct IRTargetIntrinsicDecoration : IRTargetSpecificDecoration
{
    enum { kOp = kIROp_TargetIntrinsicDecoration };
    IR_LEAF_ISA(TargetIntrinsicDecoration)

    IRStringLit* getDefinitionOperand() { return cast<IRStringLit>(getOperand(1)); }

    UnownedStringSlice getDefinition()
    {
        return getDefinitionOperand()->getStringSlice();
    }
};

struct IRGLSLOuterArrayDecoration : IRDecoration
{
    enum { kOp = kIROp_GLSLOuterArrayDecoration };
    IR_LEAF_ISA(GLSLOuterArrayDecoration)

    IRStringLit* getOuterArraynameOperand() { return cast<IRStringLit>(getOperand(0)); }

    UnownedStringSlice getOuterArrayName()
    {
        return getOuterArraynameOperand()->getStringSlice();
    }
};

// A decoration that marks a field key as having been associated
// with a particular simple semantic (e.g., `COLOR` or `SV_Position`,
// but not a `register` semantic).
//
// This is currently needed so that we can round-trip HLSL `struct`
// types that get used for varying input/output. This is an unfortunate
// case where some amount of "layout" information can't just come
// in via the `TypeLayout` part of things.
//
struct IRSemanticDecoration : IRDecoration
{
    enum { kOp = kIROp_SemanticDecoration };
    IR_LEAF_ISA(SemanticDecoration)

    IRStringLit* getSemanticNameOperand() { return cast<IRStringLit>(getOperand(0)); }

    UnownedStringSlice getSemanticName()
    {
        return getSemanticNameOperand()->getStringSlice();
    }
};

enum class IRInterpolationMode
{
    Linear,
    NoPerspective,
    NoInterpolation,

    Centroid,
    Sample,
};

struct IRInterpolationModeDecoration : IRDecoration
{
    enum { kOp = kIROp_InterpolationModeDecoration };
    IR_LEAF_ISA(InterpolationModeDecoration)

    IRConstant* getModeOperand() { return cast<IRConstant>(getOperand(0)); }

    IRInterpolationMode getMode()
    {
        return IRInterpolationMode(getModeOperand()->value.intVal);
    }
};

/// A decoration that provides a desired name to be used
/// in conjunction with the given instruction. Back-end
/// code generation may use this to help derive symbol
/// names, emit debug information, etc.
struct IRNameHintDecoration : IRDecoration
{
    enum { kOp = kIROp_NameHintDecoration };
    IR_LEAF_ISA(NameHintDecoration)

    IRStringLit* getNameOperand() { return cast<IRStringLit>(getOperand(0)); }

    UnownedStringSlice getName()
    {
        return getNameOperand()->getStringSlice();
    }
};

#define IR_SIMPLE_DECORATION(NAME)      \
    struct IR##NAME : IRDecoration      \
    {                                   \
        enum { kOp = kIROp_##NAME };    \
    IR_LEAF_ISA(NAME)                   \
    };                                  \
    /**/

/// A decoration that indicates that a variable represents
/// a vulkan ray payload, and should have a location assigned
/// to it.
IR_SIMPLE_DECORATION(VulkanRayPayloadDecoration)

/// A decoration that indicates that a variable represents
/// a vulkan callable shader payload, and should have a location assigned
/// to it.
IR_SIMPLE_DECORATION(VulkanCallablePayloadDecoration)

/// A decoration that indicates that a variable represents
/// vulkan hit attributes, and should have a location assigned
/// to it.
IR_SIMPLE_DECORATION(VulkanHitAttributesDecoration)

struct IRRequireGLSLVersionDecoration : IRDecoration
{
    enum { kOp = kIROp_RequireGLSLVersionDecoration };
    IR_LEAF_ISA(RequireGLSLVersionDecoration)

    IRConstant* getLanguageVersionOperand() { return cast<IRConstant>(getOperand(0)); }

    Int getLanguageVersion()
    {
        return Int(getLanguageVersionOperand()->value.intVal);
    }
};

struct IRRequireGLSLExtensionDecoration : IRDecoration
{
    enum { kOp = kIROp_RequireGLSLExtensionDecoration };
    IR_LEAF_ISA(RequireGLSLExtensionDecoration)

    IRStringLit* getExtensionNameOperand() { return cast<IRStringLit>(getOperand(0)); }

    UnownedStringSlice getExtensionName()
    {
        return getExtensionNameOperand()->getStringSlice();
    }
};

IR_SIMPLE_DECORATION(ReadNoneDecoration)
IR_SIMPLE_DECORATION(EarlyDepthStencilDecoration)
IR_SIMPLE_DECORATION(GloballyCoherentDecoration)

    /// A decoration that marks a value as having linkage.
    ///
    /// A value with linkage is either exported from its module,
    /// or will have a definition imported from another module.
    /// In either case, it requires a mangled name to use when
    /// matching imports and exports.
    ///
struct IRLinkageDecoration : IRDecoration
{
    IR_PARENT_ISA(LinkageDecoration)

    IRStringLit* getMangledNameOperand() { return cast<IRStringLit>(getOperand(0)); }

    UnownedStringSlice getMangledName()
    {
        return getMangledNameOperand()->getStringSlice();
    }
};

struct IRImportDecoration : IRLinkageDecoration
{
    enum { kOp = kIROp_ImportDecoration };
    IR_LEAF_ISA(ImportDecoration)
};

struct IRExportDecoration : IRLinkageDecoration
{
    enum { kOp = kIROp_ExportDecoration };
    IR_LEAF_ISA(ExportDecoration)
};

// An instruction that specializes another IR value
// (representing a generic) to a particular set of generic arguments 
// (instructions representing types, witness tables, etc.)
struct IRSpecialize : IRInst
{
    // The "base" for the call is the generic to be specialized
    IRUse base;
    IRInst* getBase() { return getOperand(0); }

    // after the generic value come the arguments
    UInt getArgCount() { return getOperandCount() - 1; }
    IRInst* getArg(UInt index) { return getOperand(index + 1); }

    IR_LEAF_ISA(Specialize)
};

// An instruction that looks up the implementation
// of an interface operation identified by `requirementDeclRef`
// in the witness table `witnessTable` which should
// hold the conformance information for a specific type.
struct IRLookupWitnessMethod : IRInst
{
    IRUse witnessTable;
    IRUse requirementKey;

    IRInst* getWitnessTable() { return witnessTable.get(); }
    IRInst* getRequirementKey() { return requirementKey.get(); }
};

struct IRLookupWitnessTable : IRInst
{
    IRUse sourceType;
    IRUse interfaceType;
};

//

struct IRCall : IRInst
{
    IR_LEAF_ISA(Call)

    IRInst* getCallee() { return getOperand(0); }

    UInt getArgCount() { return getOperandCount() - 1; }
    IRInst* getArg(UInt index) { return getOperand(index + 1); }
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
{
    IR_PARENT_ISA(Unreachable);
};

struct IRMissingReturn : IRUnreachable
{
    IR_LEAF_ISA(MissingReturn);
};

struct IRBlock;

struct IRUnconditionalBranch : IRTerminatorInst
{
    IRUse block;

    IRBlock* getTargetBlock() { return (IRBlock*)block.get(); }

    UInt getArgCount();
    IRUse* getArgs();
    IRInst* getArg(UInt index);

    IR_PARENT_ISA(UnconditionalBranch);
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
    IR_PARENT_ISA(ConditionalBranch)

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
    IR_LEAF_ISA(Switch);

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

struct IRSwizzle : IRInst
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

struct IRSwizzleSet : IRInst
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

struct IRSwizzledStore : IRInst
{
    IRInst* getDest() { return getOperand(0); }
    IRInst* getSource() { return getOperand(1); }
    UInt getElementCount()
    {
        return getOperandCount() - 2;
    }
    IRInst* getElementIndex(UInt index)
    {
        return getOperand(index + 2);
    }

    IR_LEAF_ISA(SwizzledStore)
};


struct IRPatchConstantFuncDecoration : IRDecoration
{
    enum { kOp = kIROp_PatchConstantFuncDecoration };
    IR_LEAF_ISA(PatchConstantFuncDecoration)

    IRInst* getFunc() { return getOperand(0); }
}; 

// An IR `var` instruction conceptually represents
// a stack allocation of some memory.
struct IRVar : IRInst
{
    IRPtrType* getDataType()
    {
        return cast<IRPtrType>(IRInst::getDataType());
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
    IRPtrType* getDataType()
    {
        return cast<IRPtrType>(IRInst::getDataType());
    }
};

/// @brief A global constant.
///
/// Represents a global-scope constant value in the IR.
/// The initializer for the constant is represented by
/// the code in the basic block(s) nested in this value.
struct IRGlobalConstant : IRGlobalValueWithCode
{
    IR_LEAF_ISA(GlobalConstant)
};

struct IRGlobalParam : IRInst
{
    IR_LEAF_ISA(GlobalParam)
};


// An entry in a witness table (see below)
struct IRWitnessTableEntry : IRInst
{
    // The AST-level requirement
    IRUse requirementKey;

    // The IR-level value that satisfies the requirement
    IRUse satisfyingVal;

    IRInst* getRequirementKey() { return getOperand(0); }
    IRInst* getSatisfyingVal()  { return getOperand(1); }

    IR_LEAF_ISA(WitnessTableEntry)
};

// A witness table is a global value that stores
// information about how a type conforms to some
// interface. It basically takes the form of a
// map from the required members of the interface
// to the IR values that satisfy those requirements.
struct IRWitnessTable : IRInst
{
    IRInstList<IRWitnessTableEntry> getEntries()
    {
        return IRInstList<IRWitnessTableEntry>(getChildren());
    }

    IR_LEAF_ISA(WitnessTable)
};

// An instruction that yields an undefined value.
//
// Note that we make this an instruction rather than a value,
// so that we will be able to identify a variable that is
// used when undefined.
struct IRUndefined : IRInst
{
};

// A global-scope generic parameter (a type parameter, a
// constraint parameter, etc.)
struct IRGlobalGenericParam : IRInst
{
    IR_LEAF_ISA(GlobalGenericParam)
};

// An instruction that binds a global generic parameter
// to a particular value.
struct IRBindGlobalGenericParam : IRInst
{
    IRGlobalGenericParam* getParam() { return cast<IRGlobalGenericParam>(getOperand(0)); }
    IRInst* getVal() { return getOperand(1); }

    IR_LEAF_ISA(BindGlobalGenericParam)
};


    /// An instruction that packs a concrete value into an existential-type "box"
struct IRMakeExistential : IRInst
{
    IRInst* getWrappedValue() { return getOperand(0); }
    IRInst* getWitnessTable() { return getOperand(1); }

    IR_LEAF_ISA(MakeExistential)
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

    bool operator==(const IRConstantKey& rhs) const { return inst->equal(*rhs.inst); }
    int GetHashCode() const { return inst->getHashCode(); }
};

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

    // The current parent being inserted into (this might
    // be the global scope, a function, a block inside
    // a function, etc.)
    IRInst*   insertIntoParent = nullptr;
    //
    // An instruction in the current parent that we should insert before
    IRInst*         insertBeforeInst = nullptr;

    // Get the current basic block we are inserting into (if any)
    IRBlock*                getBlock();

    // Get the current function (or other value with code)
    // that we are inserting into (if any).
    IRGlobalValueWithCode*  getFunc();

    void setInsertInto(IRInst* insertInto);
    void setInsertBefore(IRInst* insertBefore);

    IRBuilderSourceLocRAII* sourceLocInfo = nullptr;

    void addInst(IRInst* inst);

    IRInst* getBoolValue(bool value);
    IRInst* getIntValue(IRType* type, IRIntegerValue value);
    IRInst* getFloatValue(IRType* type, IRFloatingPointValue value);
    IRStringLit* getStringValue(const UnownedStringSlice& slice);
    IRPtrLit* getPtrValue(void* value);

    IRBasicType* getBasicType(BaseType baseType);
    IRBasicType* getVoidType();
    IRBasicType* getBoolType();
    IRBasicType* getIntType();
    IRStringType* getStringType();

    IRBasicBlockType*   getBasicBlockType();
    IRType* getWitnessTableType() { return nullptr; }
    IRType* getKeyType() { return nullptr; }

    IRTypeKind*     getTypeKind();
    IRGenericKind*  getGenericKind();

    IRPtrType*  getPtrType(IRType* valueType);
    IROutType*  getOutType(IRType* valueType);
    IRInOutType*  getInOutType(IRType* valueType);
    IRRefType*  getRefType(IRType* valueType);
    IRPtrTypeBase*  getPtrType(IROp op, IRType* valueType);

    IRArrayTypeBase* getArrayTypeBase(
        IROp    op,
        IRType* elementType,
        IRInst* elementCount);

    IRArrayType* getArrayType(
        IRType* elementType,
        IRInst* elementCount);

    IRUnsizedArrayType* getUnsizedArrayType(
        IRType* elementType);

    IRVectorType* getVectorType(
        IRType* elementType,
        IRInst* elementCount);

    IRMatrixType* getMatrixType(
        IRType* elementType,
        IRInst* rowCount,
        IRInst* columnCount);

    IRFuncType* getFuncType(
        UInt            paramCount,
        IRType* const*  paramTypes,
        IRType*         resultType);

    IRConstExprRate* getConstExprRate();
    IRGroupSharedRate* getGroupSharedRate();

    IRRateQualifiedType* getRateQualifiedType(
        IRRate* rate,
        IRType* dataType);

    // Set the data type of an instruction, while preserving
    // its rate, if any.
    void setDataType(IRInst* inst, IRType* dataType);

        /// Given an existential value, extract the underlying "real" value
    IRInst* emitExtractExistentialValue(
        IRType* type,
        IRInst* existentialValue);

        /// Given an existential value, extract the underlying "real" type
    IRType* emitExtractExistentialType(
        IRInst* existentialValue);

        /// Given an existential value, extract the witness table showing how the value conforms to the existential type.
    IRInst* emitExtractExistentialWitnessTable(
        IRInst* existentialValue);

    IRInst* emitSpecializeInst(
        IRType*         type,
        IRInst*         genericVal,
        UInt            argCount,
        IRInst* const*  args);

    IRInst* emitLookupInterfaceMethodInst(
        IRType* type,
        IRInst* witnessTableVal,
        IRInst* interfaceMethodVal);

    IRInst* emitCallInst(
        IRType*         type,
        IRInst*         func,
        UInt            argCount,
        IRInst* const*  args);

    IRInst* createIntrinsicInst(
        IRType*         type,
        IROp            op,
        UInt            argCount,
        IRInst* const*  args);

    IRInst* emitIntrinsicInst(
        IRType*         type,
        IROp            op,
        UInt            argCount,
        IRInst* const*  args);

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

    IRInst* emitMakeExistential(
        IRType* type,
        IRInst* value,
        IRInst* witnessTable);

    IRUndefined* emitUndefined(IRType* type);



    IRModule* createModule();

    IRFunc* createFunc();
    IRGlobalVar* createGlobalVar(
        IRType* valueType);
    IRGlobalConstant* createGlobalConstant(
        IRType* valueType);
    IRGlobalParam* createGlobalParam(
        IRType* valueType);
    IRWitnessTable* createWitnessTable();
    IRWitnessTableEntry* createWitnessTableEntry(
        IRWitnessTable* witnessTable,
        IRInst*        requirementKey,
        IRInst*        satisfyingVal);

    // Create an initially empty `struct` type.
    IRStructType*   createStructType();

    // Create an empty `interface` type.
    IRInterfaceType* createInterfaceType();

    // Create a global "key" to use for indexing into a `struct` type.
    IRStructKey*    createStructKey();

    // Create a field nested in a struct type, declaring that
    // the specified field key maps to a field with the specified type.
    IRStructField*  createStructField(
        IRStructType*   structType,
        IRStructKey*    fieldKey,
        IRType*         fieldType);

    IRGeneric* createGeneric();
    IRGeneric* emitGeneric();

    // Low-level operation for creating a type.
    IRType* getType(
        IROp            op,
        UInt            operandCount,
        IRInst* const*  operands);
    IRType* getType(
        IROp            op);

        /// Create an empty basic block.
        ///
        /// The created block will not be inserted into the current
        /// function; call `insertBlock()` to attach the block
        /// at an appropriate point.
        ///
    IRBlock* createBlock();

        /// Insert a block into the current function.
        ///
        /// This attaches the given `block` to the current function,
        /// and makes it the current block for
        /// new instructions that get emitted.
        ///
    void insertBlock(IRBlock* block);

        /// Emit a new block into the current function.
        ///
        /// This function is equivalent to using `createBlock()`
        /// and then `insertBlock()`.
        ///
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

    IRInst* emitSwizzledStore(
        IRInst*         dest,
        IRInst*         source,
        UInt            elementCount,
        IRInst* const*  elementIndices);

    IRInst* emitSwizzledStore(
        IRInst*         dest,
        IRInst*         source,
        UInt            elementCount,
        UInt const*     elementIndices);



    IRInst* emitReturn(
        IRInst*    val);

    IRInst* emitReturn();

    IRInst* emitDiscard();

    IRInst* emitUnreachable();
    IRInst* emitMissingReturn();

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

    IRGlobalGenericParam* emitGlobalGenericParam();

    IRBindGlobalGenericParam* emitBindGlobalGenericParam(
        IRInst* param,
        IRInst* val);

    IRDecoration* addDecoration(IRInst* value, IROp op, IRInst* const* operands, Int operandCount);

    IRDecoration* addDecoration(IRInst* value, IROp op)
    {
        return addDecoration(value, op, (IRInst* const*) nullptr, 0);
    }

    IRDecoration* addDecoration(IRInst* value, IROp op, IRInst* operand)
    {
        return addDecoration(value, op, &operand, 1);
    }

    IRDecoration* addDecoration(IRInst* value, IROp op, IRInst* operand0, IRInst* operand1)
    {
        IRInst* operands[] = { operand0, operand1 };
        return addDecoration(value, op, operands, SLANG_COUNT_OF(operands));
    }

    template <typename T>
    T* addRefObjectToFree(T* ptr)
    {
        getModule()->getObjectScopeManager()->addMaybeNull(ptr);
        return ptr;
    }

    template<typename T>
    void addSimpleDecoration(IRInst* value)
    {
        addDecoration(value, IROp(T::kOp), (IRInst* const*) nullptr, 0);
    }

    void addHighLevelDeclDecoration(IRInst* value, Decl* decl);
    void addLayoutDecoration(IRInst* value, Layout* layout);

    void addNameHintDecoration(IRInst* value, IRStringLit* name)
    {
        addDecoration(value, kIROp_NameHintDecoration, name);
    }

    void addNameHintDecoration(IRInst* value, UnownedStringSlice const& text)
    {
        addNameHintDecoration(value, getStringValue(text));
    }

    void addGLSLOuterArrayDecoration(IRInst* value, UnownedStringSlice const& text)
    {
        addDecoration(value, kIROp_GLSLOuterArrayDecoration, getStringValue(text));
    }

    void addInterpolationModeDecoration(IRInst* value, IRInterpolationMode mode)
    {
        addDecoration(value, kIROp_InterpolationModeDecoration, getIntValue(getIntType(), IRIntegerValue(mode)));
    }

    void addLoopControlDecoration(IRInst* value, IRLoopControl mode)
    {
        addDecoration(value, kIROp_LoopControlDecoration, getIntValue(getIntType(), IRIntegerValue(mode)));
    }

    void addSemanticDecoration(IRInst* value, UnownedStringSlice const& text)
    {
        addDecoration(value, kIROp_SemanticDecoration, getStringValue(text));
    }

    void addTargetIntrinsicDecoration(IRInst* value, UnownedStringSlice const& target, UnownedStringSlice const& definition)
    {
        addDecoration(value, kIROp_TargetIntrinsicDecoration, getStringValue(target), getStringValue(definition));
    }

    void addTargetDecoration(IRInst* value, UnownedStringSlice const& target)
    {
        addDecoration(value, kIROp_TargetDecoration, getStringValue(target));
    }

    void addRequireGLSLExtensionDecoration(IRInst* value, UnownedStringSlice const& extensionName)
    {
        addDecoration(value, kIROp_RequireGLSLExtensionDecoration, getStringValue(extensionName));
    }

    void addRequireGLSLVersionDecoration(IRInst* value, Int version)
    {
        addDecoration(value, kIROp_RequireGLSLVersionDecoration, getIntValue(getIntType(), IRIntegerValue(version)));
    }

    void addPatchConstantFuncDecoration(IRInst* value, IRInst* patchConstantFunc)
    {
        addDecoration(value, kIROp_PatchConstantFuncDecoration, patchConstantFunc);
    }

    void addImportDecoration(IRInst* value, UnownedStringSlice const& mangledName)
    {
        addDecoration(value, kIROp_ImportDecoration, getStringValue(mangledName));
    }

    void addExportDecoration(IRInst* value, UnownedStringSlice const& mangledName)
    {
        addDecoration(value, kIROp_ExportDecoration, getStringValue(mangledName));
    }

    void addEntryPointDecoration(IRInst* value)
    {
        addDecoration(value, kIROp_EntryPointDecoration);
    }

        /// Add a decoration that indicates that the given `inst` depends on the given `dependency`.
        ///
        /// This decoration can be used to ensure that a value that an instruction
        /// implicitly depends on cannot be eliminated so long as the instruction
        /// itself is kept alive.
        ///
    void addDependsOnDecoration(IRInst* inst, IRInst* dependency)
    {
        addDecoration(inst, kIROp_DependsOnDecoration, dependency);
    }
};

void addHoistableInst(
    IRBuilder*  builder,
    IRInst*     inst);

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
        SLANG_ASSERT(builder->sourceLocInfo == this);
        builder->sourceLocInfo = next;
    }
};

//

void markConstExpr(
    IRBuilder*  builder,
    IRInst*     irValue);

//

IRTargetIntrinsicDecoration* findTargetIntrinsicDecoration(
        IRInst*        val,
        String const&   targetName);

}

#endif
