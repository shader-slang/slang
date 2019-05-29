// slang-c-like-source-emitter.h
#ifndef SLANG_C_LIKE_SOURCE_EMITTER_H_INCLUDED
#define SLANG_C_LIKE_SOURCE_EMITTER_H_INCLUDED

#include "../core/basic.h"

#include "compiler.h"

#include "slang-emit-context.h"
#include "slang-extension-usage-tracker.h"

#include "ir.h"
#include "ir-insts.h"
#include "ir-restructure.h"

namespace Slang
{

enum EPrecedence
{
#define LEFT(NAME)                      \
    kEPrecedence_##NAME##_Left,     \
    kEPrecedence_##NAME##_Right

#define RIGHT(NAME)                     \
    kEPrecedence_##NAME##_Right,    \
    kEPrecedence_##NAME##_Left

#define NONASSOC(NAME)                  \
    kEPrecedence_##NAME##_Left,     \
    kEPrecedence_##NAME##_Right = kEPrecedence_##NAME##_Left

    NONASSOC(None),
    LEFT(Comma),

    NONASSOC(General),

    RIGHT(Assign),

    RIGHT(Conditional),

    LEFT(Or),
    LEFT(And),
    LEFT(BitOr),
    LEFT(BitXor),
    LEFT(BitAnd),

    LEFT(Equality),
    LEFT(Relational),
    LEFT(Shift),
    LEFT(Additive),
    LEFT(Multiplicative),
    RIGHT(Prefix),
    LEFT(Postfix),
    NONASSOC(Atomic),
};

// Info on an op for emit purposes
struct EOpInfo
{
    char const* op;
    EPrecedence leftPrecedence;
    EPrecedence rightPrecedence;
};

struct CLikeSourceEmitter
{
    enum class BuiltInCOp
    {
        Splat,                  //< Splat a single value to all values of a vector or matrix type
        Init,                   //< Initialize with parameters (must match the type)
    };

    typedef unsigned int ESemanticMask;
    enum
    {
        kESemanticMask_None = 0,
        kESemanticMask_NoPackOffset = 1 << 0,
        kESemanticMask_Default = kESemanticMask_NoPackOffset,
    };

    // Hack to allow IR emit for global constant to override behavior
    enum class IREmitMode
    {
        Default,
        GlobalConstant,
    };

    struct EmitVarChain;
    struct IRDeclaratorInfo;
    struct EDeclarator;
    struct ComputeEmitActionsContext;

    // An action to be performed during code emit.
    struct EmitAction
    {
        enum Level
        {
            ForwardDeclaration,
            Definition,
        };
        Level   level;
        IRInst* inst;
    };

        /// Ctor
    CLikeSourceEmitter(EmitContext* context);

        /// Get the source manager
    SourceManager* getSourceManager() { return m_context->getSourceManager(); }

        /// Get the diagnostic sink
    DiagnosticSink* getSink() { return m_context->getSink();}

    //
    // Types
    //

    void emitDeclarator(EDeclarator* declarator);

    void emitGLSLTypePrefix(IRType* type, bool promoteHalfToFloat = false);

    void emitHLSLTextureType(IRTextureTypeBase* texType);

    void emitGLSLTextureOrTextureSamplerType(IRTextureTypeBase*  type, char const* baseName);

    void emitGLSLTextureType(IRTextureType* texType);

    void emitGLSLTextureSamplerType(IRTextureSamplerType* type);
    
    void emitGLSLImageType(IRGLSLImageType* type);

    void emitTextureType(IRTextureType* texType);

    void emitTextureSamplerType(IRTextureSamplerType* type);
    void emitImageType(IRGLSLImageType* type);

    void emitVectorTypeName(IRType* elementType, IRIntegerValue elementCount);

    void emitVectorTypeImpl(IRVectorType* vecType);

    void emitMatrixTypeImpl(IRMatrixType* matType);

    void emitSamplerStateType(IRSamplerStateTypeBase* samplerStateType);

    void emitStructuredBufferType(IRHLSLStructuredBufferTypeBase* type);

    void emitUntypedBufferType(IRUntypedBufferResourceType* type);

    void emitSimpleTypeImpl(IRType* type);

    void emitArrayTypeImpl(IRArrayType* arrayType, EDeclarator* declarator);

    void emitUnsizedArrayTypeImpl(IRUnsizedArrayType* arrayType, EDeclarator* declarator);

    void emitTypeImpl(IRType* type, EDeclarator* declarator);

    void emitType(
        IRType*             type,
        SourceLoc const&    typeLoc,
        Name*               name,
        SourceLoc const&    nameLoc);

    void emitType(IRType* type, Name* name);

    void emitType(IRType* type, String const& name);

    void emitType(IRType* type);

    //
    // Expressions
    //

    bool maybeEmitParens(EOpInfo& outerPrec, EOpInfo prec);

    void maybeCloseParens(bool needClose);

    bool isTargetIntrinsicModifierApplicable(String const& targetName);

    void emitType(IRType* type, Name* name, SourceLoc const& nameLoc);

    void emitType(IRType* type, NameLoc const& nameAndLoc);

    bool isTargetIntrinsicModifierApplicable(IRTargetIntrinsicDecoration* decoration);

    void emitStringLiteral(const String& value);

    static EOpInfo leftSide(EOpInfo const& outerPrec, EOpInfo const& prec);
    static EOpInfo rightSide(EOpInfo const& prec, EOpInfo const& outerPrec);

    void requireGLSLExtension(const String& name);

    void requireGLSLVersion(ProfileVersion version);
    void requireGLSLVersion(int version);
    void setSampleRateFlag();

    void doSampleRateInputCheck(Name* name);

    void emitVal(IRInst* val, const EOpInfo&  outerPrec);

    UInt getBindingOffset(EmitVarChain* chain, LayoutResourceKind kind);
    UInt getBindingSpace(EmitVarChain* chain, LayoutResourceKind kind);

        // Emit a single `register` semantic, as appropriate for a given resource-type-specific layout info
        // Keyword to use in the uniform case (`register` for globals, `packoffset` inside a `cbuffer`)
    void emitHLSLRegisterSemantic(
        LayoutResourceKind  kind,
        EmitVarChain*       chain,
        char const* uniformSemanticSpelling = "register");

        // Emit all the `register` semantics that are appropriate for a particular variable layout
    void emitHLSLRegisterSemantics(EmitVarChain* chain, char const* uniformSemanticSpelling = "register");
    void emitHLSLRegisterSemantics(VarLayout* varLayout, char const* uniformSemanticSpelling = "register");

    void emitHLSLParameterGroupFieldLayoutSemantics(EmitVarChain*       chain);

    void emitHLSLParameterGroupFieldLayoutSemantics(RefPtr<VarLayout> fieldLayout, EmitVarChain* inChain);

    bool emitGLSLLayoutQualifier(LayoutResourceKind  kind, EmitVarChain*       chain);

    void emitGLSLLayoutQualifiers(
        RefPtr<VarLayout>               layout,
        EmitVarChain*                   inChain,
        LayoutResourceKind              filter = LayoutResourceKind::None);

    void emitGLSLVersionDirective();

    void emitGLSLPreprocessorDirectives();

    /// Emit directives to control overall layout computation for the emitted code.
    void emitLayoutDirectives(TargetRequest* targetReq);

        // Utility code for generating unique IDs as needed
        // during the emit process (e.g., for declarations
        // that didn't originally have names, but now need to).
    UInt allocateUniqueID();

    // IR-level emit logic

    UInt getID(IRInst* value);

    /// "Scrub" a name so that it complies with restrictions of the target language.
    String scrubName(const String& name);

    String generateIRName(IRInst* inst);
    String getIRName(IRInst* inst);

    void emitDeclarator(EmitContext* ctx, IRDeclaratorInfo* declarator);    
    void emitIRSimpleValue(EmitContext* /*context*/, IRInst* inst);

    CodeGenTarget getTarget(EmitContext* ctx);

    bool shouldFoldIRInstIntoUseSites(
        EmitContext*    ctx,
        IRInst*        inst,
        IREmitMode      mode);

    void emitIROperand(
        EmitContext*    ctx,
        IRInst*         inst,
        IREmitMode      mode,
        EOpInfo const&  outerPrec);

    void emitIRArgs(
        EmitContext*    ctx,
        IRInst*         inst,
        IREmitMode      mode);

    void emitIRType(
        EmitContext*    /*context*/,
        IRType*         type,
        String const&   name);

    void emitIRType(
        EmitContext*    /*context*/,
        IRType*         type,
        Name*           name);

    void emitIRType(EmitContext* /*context*/, IRType* type);

    void emitIRRateQualifiers(EmitContext* ctx, IRRate* rate);

    void emitIRRateQualifiers(EmitContext*    ctx, IRInst* value);

    void emitIRInstResultDecl(EmitContext* ctx, IRInst* inst);

    IRTargetIntrinsicDecoration* findTargetIntrinsicDecoration(EmitContext* /* ctx */, IRInst* inst);

    // Check if the string being used to define a target intrinsic
    // is an "ordinary" name, such that we can simply emit a call
    // to the new name with the arguments of the old operation.
    static bool isOrdinaryName(const String& name);
    
    void emitTargetIntrinsicCallExpr(
        EmitContext*                    ctx,
        IRCall*                         inst,
        IRFunc*                         /* func */,
        IRTargetIntrinsicDecoration*    targetIntrinsic,
        IREmitMode                      mode,
        EOpInfo const&                  inOuterPrec);

    void emitIntrinsicCallExpr(
        EmitContext*    ctx,
        IRCall*         inst,
        IRFunc*         func,
        IREmitMode      mode,
        EOpInfo const&  inOuterPrec);

    void emitIRCallExpr(
        EmitContext*    ctx,
        IRCall*         inst,
        IREmitMode      mode,
        EOpInfo         outerPrec);

    
    void emitNot(EmitContext* ctx, IRInst* inst, IREmitMode mode, EOpInfo& ioOuterPrec, bool* outNeedClose);

    void emitComparison(EmitContext* ctx, IRInst* inst, IREmitMode mode, EOpInfo& ioOuterPrec, const EOpInfo& opPrec, bool* needCloseOut);

    void emitIRInstExpr(
        EmitContext*    ctx,
        IRInst*         inst,
        IREmitMode      mode,
        EOpInfo const&  inOuterPrec);
    
    BaseType extractBaseType(IRType* inType);

    void emitIRInst(
        EmitContext*    ctx,
        IRInst*         inst,
        IREmitMode      mode);

    void emitIRInstImpl(
        EmitContext*    ctx,
        IRInst*         inst,
        IREmitMode      mode);

    void emitIRSemantics(EmitContext*, VarLayout* varLayout);

    void emitIRSemantics(EmitContext* ctx, IRInst* inst);

    VarLayout* getVarLayout(EmitContext*    /*context*/, IRInst* var);

    void emitIRLayoutSemantics(
        EmitContext*    ctx,
        IRInst*         inst,
        char const*     uniformSemanticSpelling = "register");

    // When we are about to traverse an edge from one block to another,
    // we need to emit the assignments that conceptually occur "along"
    // the edge. In traditional SSA these are the phi nodes in the
    // target block, while in our representation these use the arguments
    // to the branch instruction to fill in the parameters of the target.
    void emitPhiVarAssignments(
        EmitContext*    ctx,
        UInt            argCount,
        IRUse*          args,
        IRBlock*        targetBlock);

    /// Emit high-level language statements from a structured region.
    void emitRegion(
        EmitContext*    ctx,
        Region*         inRegion);

    
    /// Emit high-level language statements from a structured region tree.
    void emitRegionTree(
        EmitContext*    ctx,
        RegionTree*     regionTree);

    // Is an IR function a definition? (otherwise it is a declaration)
    bool isDefinition(IRFunc* func);

    String getIRFuncName(IRFunc* func);

    void emitAttributeSingleString(const char* name, FuncDecl* entryPoint, Attribute* attrib);

    void emitAttributeSingleInt(const char* name, FuncDecl* entryPoint, Attribute* attrib);

    void emitFuncDeclPatchConstantFuncAttribute(IRFunc* irFunc, FuncDecl* entryPoint, PatchConstantFuncAttribute* attrib);

    void emitIREntryPointAttributes_HLSL(
        IRFunc*             irFunc,
        EmitContext*        ctx,
        EntryPointLayout*   entryPointLayout);

    void emitIREntryPointAttributes_GLSL(
        IRFunc*             irFunc,
        EmitContext*        /*ctx*/,
        EntryPointLayout*   entryPointLayout);

    void emitIREntryPointAttributes(
        IRFunc*             irFunc,
        EmitContext*        ctx,
        EntryPointLayout*   entryPointLayout);

    void emitPhiVarDecls(EmitContext* ctx, IRFunc* func);

    /// Emit high-level statements for the body of a function.
    void emitIRFunctionBody(EmitContext* ctx, IRGlobalValueWithCode* code);

    void emitIRSimpleFunc(EmitContext* ctx, IRFunc* func);

    void emitIRParamType(
        EmitContext*    ctx,
        IRType*         type,
        String const&   name);

    IRInst* getSpecializedValue(IRSpecialize* specInst);

    void emitIRFuncDecl(EmitContext* ctx, IRFunc* func);

    EntryPointLayout* getEntryPointLayout(EmitContext*    /*context*/, IRFunc*         func);

    EntryPointLayout* asEntryPoint(IRFunc* func);

        // Detect if the given IR function represents a
        // declaration of an intrinsic/builtin for the
        // current code-generation target.
    bool isTargetIntrinsic(EmitContext*    /*ctxt*/, IRFunc* func);

        // Check whether a given value names a target intrinsic,
        // and return the IR function representing the intrinsic
        // if it does.
    IRFunc* asTargetIntrinsic(EmitContext* ctxt, IRInst* value);

    void emitIRFunc(EmitContext* ctx, IRFunc* func);

    void emitIRStruct(EmitContext* ctx, IRStructType* structType);

    void emitIRMatrixLayoutModifiers(EmitContext* ctx, VarLayout* layout);

        // Emit the `flat` qualifier if the underlying type
        // of the variable is an integer type.
    void maybeEmitGLSLFlatModifier(EmitContext*, IRType* valueType);

    void emitInterpolationModifiers(
        EmitContext*    ctx,
        IRInst*         varInst,
        IRType*         valueType,
        VarLayout*      layout);

    UInt getRayPayloadLocation(EmitContext* ctx, IRInst* inst);

    UInt getCallablePayloadLocation(EmitContext* ctx, IRInst* inst);

    void emitGLSLImageFormatModifier(IRInst* var, IRTextureType*  resourceType);

        /// Emit modifiers that should apply even for a declaration of an SSA temporary.
    void emitIRTempModifiers(EmitContext* ctx, IRInst* temp);

    void emitIRVarModifiers(
        EmitContext*    ctx,
        VarLayout*      layout,
        IRInst*         varDecl,
        IRType*         varType);

    void emitHLSLParameterGroup(
        EmitContext*                    ctx,
        IRGlobalParam*                  varDecl,
        IRUniformParameterGroupType*    type);

        /// Emit the array brackets that go on the end of a declaration of the given type.
    void emitArrayBrackets(EmitContext* ctx, IRType* inType);

    void emitGLSLParameterGroup(
        EmitContext*                    ctx,
        IRGlobalParam*                  varDecl,
        IRUniformParameterGroupType*    type);
    
    void emitIRParameterGroup(
        EmitContext*                    ctx,
        IRGlobalParam*                  varDecl,
        IRUniformParameterGroupType*    type);

    void emitIRVar(EmitContext* ctx, IRVar* varDecl);

    void emitIRStructuredBuffer_GLSL(
        EmitContext*                    ctx,
        IRGlobalParam*                  varDecl,
        IRHLSLStructuredBufferTypeBase* structuredBufferType);
    
    void emitIRByteAddressBuffer_GLSL(
        EmitContext*                    ctx,
        IRGlobalParam*                  varDecl,
        IRByteAddressBufferTypeBase*    byteAddressBufferType);

    void emitIRGlobalVar(EmitContext* ctx, IRGlobalVar* varDecl);
    void emitIRGlobalParam(EmitContext* ctx, IRGlobalParam* varDecl);
    void emitIRGlobalConstantInitializer(EmitContext* ctx, IRGlobalConstant* valDecl);

    void emitIRGlobalConstant(EmitContext* ctx, IRGlobalConstant* valDecl);

    void emitIRGlobalInst(EmitContext* ctx, IRInst* inst);

    void ensureInstOperand(
        ComputeEmitActionsContext*  ctx,
        IRInst*                     inst,
        EmitAction::Level           requiredLevel = EmitAction::Level::Definition);

    void ensureInstOperandsRec(ComputeEmitActionsContext* ctx, IRInst* inst);

    void ensureGlobalInst(
        ComputeEmitActionsContext*  ctx,
        IRInst*                     inst,
        EmitAction::Level           requiredLevel);

    void computeIREmitActions(IRModule* module, List<EmitAction>& ioActions);

    void executeIREmitActions(EmitContext* ctx, List<EmitAction> const& actions);
    void emitIRModule(EmitContext* ctx, IRModule* module);

    protected:

    void _requireHalf();
    void _emitCVecType(IROp op, Int size);
    void _emitCMatType(IROp op, IRIntegerValue rowCount, IRIntegerValue colCount);

    void _emitCFunc(BuiltInCOp cop, IRType* type);
    void _maybeEmitGLSLCast(EmitContext* ctx, IRType* castType, IRInst* inst, IREmitMode mode);


    EmitContext* m_context;
    SourceStream* m_stream;
};
    
}
#endif
