// slang-emit-c-like.h
#ifndef SLANG_EMIT_C_LIKE_H
#define SLANG_EMIT_C_LIKE_H

#include "../core/slang-basic.h"

#include "slang-compiler.h"

#include "slang-emit-glsl-extension-tracker.h"
#include "slang-emit-precedence.h"
#include "slang-emit-source-writer.h"

#include "slang-ir.h"
#include "slang-ir-insts.h"
#include "slang-ir-restructure.h"

namespace Slang
{

class CLikeSourceEmitter: public RefObject
{
public:
    struct Desc
    {
        BackEndCompileRequest* compileRequest = nullptr;
            // The target language we want to generate code for
        CodeGenTarget target = CodeGenTarget::Unknown;
            // The entry point we are being asked to compile
        EntryPoint* entryPoint = nullptr;
            // The "effective" profile that is being used to emit code,
            // combining information from the target and entry point.
        Profile effectiveProfile = Profile::RawEnum::Unknown;

        SourceWriter* sourceWriter = nullptr;
            // The layout for the entry point
        EntryPointLayout* entryPointLayout = nullptr;

        ProgramLayout* programLayout = nullptr;
            // We track the original global-scope layout so that we can
            // find layout information for `import`ed parameters.
            //
            // TODO: This will probably change if we represent imports
            // explicitly in the layout data.
        StructTypeLayout* globalStructLayout = nullptr;
    };
    
        /// To simplify cases 
    enum class SourceStyle
    {
        Unknown,
        GLSL,
        HLSL,
        C,
        CPP,
        CountOf,
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

    // A chain of variables to use for emitting semantic/layout info
    struct EmitVarChain
    {
        VarLayout*      varLayout;
        EmitVarChain*   next;

        EmitVarChain()
            : varLayout(0)
            , next(0)
        {}

        EmitVarChain(VarLayout* varLayout)
            : varLayout(varLayout)
            , next(0)
        {}

        EmitVarChain(VarLayout* varLayout, EmitVarChain* next)
            : varLayout(varLayout)
            , next(next)
        {}
    };

        /// Ctor
    CLikeSourceEmitter(const Desc& desc);
    
        /// Get the source manager
    SourceManager* getSourceManager() { return m_compileRequest->getSourceManager(); }

        /// Get the diagnostic sink
    DiagnosticSink* getSink() { return m_compileRequest->getSink();}
    LineDirectiveMode getLineDirectiveMode() { return m_compileRequest->getLineDirectiveMode(); }

        /// Get the code gen target
    CodeGenTarget getTarget() { return m_target; }
        /// Get the source style
    SLANG_FORCE_INLINE SourceStyle getSourceStyle() const { return m_sourceStyle;  }

    void noteInternalErrorLoc(SourceLoc loc) { return getSink()->noteInternalErrorLoc(loc); }

    GLSLExtensionTracker* getGLSLExtensionTracker() { return &m_glslExtensionTracker;  }

    //
    // Types
    //

    void emitDeclarator(EDeclarator* declarator);

    void emitTextureType(IRTextureType* texType);

    void emitTextureSamplerType(IRTextureSamplerType* type);
    void emitImageType(IRGLSLImageType* type);

    void emitVectorTypeName(IRType* elementType, IRIntegerValue elementCount);

    void emitSamplerStateType(IRSamplerStateTypeBase* samplerStateType);

    void emitStructuredBufferType(IRHLSLStructuredBufferTypeBase* type);

    void emitUntypedBufferType(IRUntypedBufferResourceType* type);

    void emitType(IRType* type, const SourceLoc& typeLoc, Name* name, const SourceLoc& nameLoc);
    void emitType(IRType* type, Name* name);
    void emitType(IRType* type, String const& name);
    void emitType(IRType* type);

    //
    // Expressions
    //

    bool maybeEmitParens(EmitOpInfo& outerPrec, const EmitOpInfo& prec);

    void maybeCloseParens(bool needClose);

    bool isTargetIntrinsicModifierApplicable(String const& targetName);

    void emitType(IRType* type, Name* name, SourceLoc const& nameLoc);

    void emitType(IRType* type, NameLoc const& nameAndLoc);

    bool isTargetIntrinsicModifierApplicable(IRTargetIntrinsicDecoration* decoration);

    void emitStringLiteral(const String& value);

    void setSampleRateFlag();

    void doSampleRateInputCheck(Name* name);

    void emitVal(IRInst* val, const EmitOpInfo& outerPrec);

    UInt getBindingOffset(EmitVarChain* chain, LayoutResourceKind kind);
    UInt getBindingSpace(EmitVarChain* chain, LayoutResourceKind kind);

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

    void emitDeclarator(IRDeclaratorInfo* declarator);    
    void emitIRSimpleValue(IRInst* inst);
    
    bool shouldFoldIRInstIntoUseSites(IRInst* inst, IREmitMode mode);

    void emitIROperand(IRInst* inst, IREmitMode mode, EmitOpInfo const& outerPrec);

    void emitIRArgs(IRInst* inst, IREmitMode mode);

    void emitIRType(IRType* type, String const&   name);

    void emitIRType(IRType* type, Name* name);

    void emitIRType(IRType* type);

    void emitIRRateQualifiers(IRInst* value);

    void emitIRInstResultDecl(IRInst* inst);

    IRTargetIntrinsicDecoration* findTargetIntrinsicDecoration(IRInst* inst);

    // Check if the string being used to define a target intrinsic
    // is an "ordinary" name, such that we can simply emit a call
    // to the new name with the arguments of the old operation.
    static bool isOrdinaryName(const String& name);
    
    void emitTargetIntrinsicCallExpr(
        IRCall*                         inst,
        IRFunc*                         /* func */,
        IRTargetIntrinsicDecoration*    targetIntrinsic,
        IREmitMode                      mode,
        EmitOpInfo const&                  inOuterPrec);

    void emitIntrinsicCallExpr(
        IRCall*         inst,
        IRFunc*         func,
        IREmitMode      mode,
        EmitOpInfo const&  inOuterPrec);

    void emitIRCallExpr(IRCall* inst, IREmitMode mode, EmitOpInfo outerPrec);

    void emitIRInstExpr(IRInst* inst, IREmitMode mode, EmitOpInfo const&  inOuterPrec);
    
    BaseType extractBaseType(IRType* inType);

    void emitIRInst(IRInst* inst, IREmitMode mode);

    void emitIRSemantics(VarLayout* varLayout);

    void emitIRSemantics(IRInst* inst);

    VarLayout* getVarLayout(IRInst* var);

    void emitIRLayoutSemantics(IRInst* inst, char const* uniformSemanticSpelling = "register");

        // When we are about to traverse an edge from one block to another,
        // we need to emit the assignments that conceptually occur "along"
        // the edge. In traditional SSA these are the phi nodes in the
        // target block, while in our representation these use the arguments
        // to the branch instruction to fill in the parameters of the target.
    void emitPhiVarAssignments(UInt argCount, IRUse* args, IRBlock* targetBlock);

        /// Emit high-level language statements from a structured region.
    void emitRegion(Region* inRegion);

        /// Emit high-level language statements from a structured region tree.
    void emitRegionTree(RegionTree* regionTree);

        // Is an IR function a definition? (otherwise it is a declaration)
    bool isDefinition(IRFunc* func);

    String getIRFuncName(IRFunc* func);

    void emitIREntryPointAttributes(IRFunc* irFunc, EntryPointLayout* entryPointLayout);

    void emitPhiVarDecls(IRFunc* func);

        /// Emit high-level statements for the body of a function.
    void emitIRFunctionBody(IRGlobalValueWithCode* code);

    void emitIRSimpleFunc(IRFunc* func);

    void emitIRParamType(IRType* type, String const& name);

    IRInst* getSpecializedValue(IRSpecialize* specInst);

    void emitIRFuncDecl(IRFunc* func);

    EntryPointLayout* getEntryPointLayout(IRFunc* func);

    EntryPointLayout* asEntryPoint(IRFunc* func);

        // Detect if the given IR function represents a
        // declaration of an intrinsic/builtin for the
        // current code-generation target.
    bool isTargetIntrinsic(IRFunc* func);

        // Check whether a given value names a target intrinsic,
        // and return the IR function representing the intrinsic
        // if it does.
    IRFunc* asTargetIntrinsic(IRInst* value);

    void emitIRFunc(IRFunc* func);

    void emitIRStruct(IRStructType* structType);

    void emitIRMatrixLayoutModifiers(VarLayout* layout);

    void emitInterpolationModifiers(IRInst* varInst, IRType* valueType, VarLayout* layout);

    UInt getRayPayloadLocation(IRInst* inst);

    UInt getCallablePayloadLocation(IRInst* inst);

        /// Emit modifiers that should apply even for a declaration of an SSA temporary.
    void emitIRTempModifiers(IRInst* temp);

    void emitIRVarModifiers(VarLayout* layout, IRInst* varDecl, IRType* varType);

        /// Emit the array brackets that go on the end of a declaration of the given type.
    void emitArrayBrackets(IRType* inType);

    void emitIRParameterGroup(IRGlobalParam* varDecl, IRUniformParameterGroupType* type);

    void emitIRVar(IRVar* varDecl);

    
    void emitIRGlobalVar(IRGlobalVar* varDecl);
    void emitIRGlobalParam(IRGlobalParam* varDecl);
    void emitIRGlobalConstantInitializer(IRGlobalConstant* valDecl);

    void emitIRGlobalConstant(IRGlobalConstant* valDecl);

    void emitIRGlobalInst(IRInst* inst);

    void ensureInstOperand(ComputeEmitActionsContext* ctx, IRInst* inst, EmitAction::Level requiredLevel = EmitAction::Level::Definition);

    void ensureInstOperandsRec(ComputeEmitActionsContext* ctx, IRInst* inst);

    void ensureGlobalInst(ComputeEmitActionsContext* ctx, IRInst* inst, EmitAction::Level requiredLevel);

    void computeIREmitActions(IRModule* module, List<EmitAction>& ioActions);

    void executeIREmitActions(List<EmitAction> const& actions);
    void emitIRModule(IRModule* module);

    void emitPreprocessorDirectives() { emitPreprocessorDirectivesImpl(); }
    void emitSimpleType(IRType* type);
  
        /// Gets a source style for a target. Returns Unknown if not a known target
    static SourceStyle getSourceStyle(CodeGenTarget target);

    protected:

    virtual void emitLayoutSemanticsImpl(IRInst* inst, char const* uniformSemanticSpelling = "register") { SLANG_UNUSED(inst); SLANG_UNUSED(uniformSemanticSpelling); }
    virtual void emitParameterGroupImpl(IRGlobalParam* varDecl, IRUniformParameterGroupType* type) = 0;
    virtual void emitEntryPointAttributesImpl(IRFunc* irFunc, EntryPointLayout* entryPointLayout) = 0;
    virtual void emitTextureTypeImpl(IRTextureType* texType);
    virtual void emitImageTypeImpl(IRGLSLImageType* type);
    virtual void emitImageFormatModifierImpl(IRInst* varDecl, IRType* varType) { SLANG_UNUSED(varDecl); SLANG_UNUSED(varType); }
    virtual void emitLayoutQualifiersImpl(VarLayout* layout) { SLANG_UNUSED(layout); }
    virtual void emitTextureSamplerTypeImpl(IRTextureSamplerType* type);
    virtual void emitTextureOrTextureSamplerTypeImpl(IRTextureTypeBase*  type, char const* baseName) { SLANG_UNUSED(type); SLANG_UNUSED(baseName); }
    virtual void emitVectorTypeNameImpl(IRType* elementType, IRIntegerValue elementCount) = 0;
    virtual void emitMatrixTypeImpl(IRMatrixType* matType) = 0;
    virtual void emitUntypedBufferTypeImpl(IRUntypedBufferResourceType* type);
    virtual void emitStructuredBufferTypeImpl(IRHLSLStructuredBufferTypeBase* type);
    virtual void emitSamplerStateTypeImpl(IRSamplerStateTypeBase* samplerStateType);        
    virtual void emitPreprocessorDirectivesImpl() {}
    virtual void emitLayoutDirectivesImpl(TargetRequest* targetReq) { SLANG_UNUSED(targetReq); }
    virtual void emitRateQualifiersImpl(IRRate* rate) { SLANG_UNUSED(rate); }
    virtual void emitSemanticsImpl(IRInst* inst) { SLANG_UNUSED(inst);  }
    virtual void emitSimpleFuncParamImpl(IRParam* param);
    virtual void emitInterpolationModifiersImpl(IRInst* varInst, IRType* valueType, VarLayout* layout) { SLANG_UNUSED(varInst); SLANG_UNUSED(valueType); SLANG_UNUSED(layout); }

    virtual void handleCallExprDecorationsImpl(IRInst* funcValue) { SLANG_UNUSED(funcValue); }

    virtual bool tryEmitSimpleTypeImpl(IRType* type);
    virtual bool tryEmitGlobalParamImpl(IRGlobalParam* varDecl, IRType* varType) { SLANG_UNUSED(varDecl); SLANG_UNUSED(varType); return false; }
    virtual bool tryEmitInstExprImpl(IRInst* inst, IREmitMode mode, const EmitOpInfo& inOuterPrec) { SLANG_UNUSED(inst); SLANG_UNUSED(mode); SLANG_UNUSED(inOuterPrec); return false; }

    void _emitArrayType(IRArrayType* arrayType, EDeclarator* declarator);
    void _emitUnsizedArrayType(IRUnsizedArrayType* arrayType, EDeclarator* declarator);
    void _emitType(IRType* type, EDeclarator* declarator);
    void _emitIRInst(IRInst* inst, IREmitMode mode);
    void _emitVectorType(IRVectorType* vecType);
    
    BackEndCompileRequest* m_compileRequest = nullptr;

    // The entry point we are being asked to compile
    EntryPoint* m_entryPoint;

    // The layout for the entry point
    EntryPointLayout* m_entryPointLayout;

    // The target language we want to generate code for
    CodeGenTarget m_target;
    // Source style - a simplification of the more nuanced m_target
    SourceStyle m_sourceStyle;

    // Where source is written to
    SourceWriter* m_writer;

    // We only want to emit each `import`ed module one time, so
    // we maintain a set of already-emitted modules.
    HashSet<ModuleDecl*> m_modulesAlreadyEmitted;

    // We track the original global-scope layout so that we can
    // find layout information for `import`ed parameters.
    //
    // TODO: This will probably change if we represent imports
    // explicitly in the layout data.
    StructTypeLayout* m_globalStructLayout;

    ProgramLayout* m_programLayout;

    ModuleDecl* m_program;

    GLSLExtensionTracker m_glslExtensionTracker;

    UInt m_uniqueIDCounter = 1;
    Dictionary<IRInst*, UInt> m_mapIRValueToID;
    Dictionary<Decl*, UInt> m_mapDeclToID;

    HashSet<String> m_irDeclsVisited;

    HashSet<String> m_irTupleTypes;

    // The "effective" profile that is being used to emit code,
    // combining information from the target and entry point.
    Profile m_effectiveProfile;

    // Map a string name to the number of times we have seen this
    // name used so far during code emission.
    Dictionary<String, UInt> m_uniqueNameCounters;

    // Map an IR instruction to the name that we've decided
    // to use for it when emitting code.
    Dictionary<IRInst*, String> m_mapInstToName;

    Dictionary<IRInst*, UInt> m_mapIRValueToRayPayloadLocation;
    Dictionary<IRInst*, UInt> m_mapIRValueToCallablePayloadLocation;
};

}
#endif
