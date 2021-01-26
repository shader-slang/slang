// slang-emit-c-like.h
#ifndef SLANG_EMIT_C_LIKE_H
#define SLANG_EMIT_C_LIKE_H

#include "../core/slang-basic.h"

#include "slang-compiler.h"

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

        /// The target language we want to generate code for
        CodeGenTarget target = CodeGenTarget::Unknown;

            /// The stage for the entry point we are being asked to compile
        Stage entryPointStage = Stage::Unknown;

            /// The "effective" profile that is being used to emit code,
            /// combining information from the target and entry point.
        Profile effectiveProfile = Profile::RawEnum::Unknown;

            /// The capabilities of the target
        CapabilitySet targetCaps;

        SourceWriter* sourceWriter = nullptr;
    };

    enum
    {
        kThreadGroupAxisCount = 3,
    };
    
    typedef unsigned int ESemanticMask;
    enum
    {
        kESemanticMask_None = 0,
        kESemanticMask_NoPackOffset = 1 << 0,
        kESemanticMask_Default = kESemanticMask_NoPackOffset,
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
        IRVarLayout*      varLayout;
        EmitVarChain*   next;

        EmitVarChain()
            : varLayout(nullptr)
            , next(nullptr)
        {}

        EmitVarChain(IRVarLayout* varLayout)
            : varLayout(varLayout)
            , next(nullptr)
        {}

        EmitVarChain(IRVarLayout* varLayout, EmitVarChain* next)
            : varLayout(varLayout)
            , next(next)
        {}
    };

        /// Must be called before used
    virtual SlangResult init();

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
    SLANG_FORCE_INLINE SourceLanguage getSourceLanguage() const { return m_sourceLanguage;  }

    void noteInternalErrorLoc(SourceLoc loc) { return getSink()->noteInternalErrorLoc(loc); }

    CapabilitySet getTargetCaps() { return m_targetCaps; }

    //
    // Types
    //

    void emitDeclarator(EDeclarator* declarator);

    void emitType(IRType* type, const StringSliceLoc* nameLoc) { emitTypeImpl(type, nameLoc); }
    void emitType(IRType* type, Name* name);
    void emitType(IRType* type, String const& name);
    void emitType(IRType* type);
    void emitType(IRType* type, Name* name, SourceLoc const& nameLoc);
    void emitType(IRType* type, NameLoc const& nameAndLoc);

    //
    // Expressions
    //

    bool maybeEmitParens(EmitOpInfo& outerPrec, const EmitOpInfo& prec);

    void maybeCloseParens(bool needClose);

    void emitStringLiteral(const String& value);

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
    void appendScrubbedName(const UnownedStringSlice& name, StringBuilder& out);

    String generateName(IRInst* inst);
    virtual String generateEntryPointNameImpl(IREntryPointDecoration* entryPointDecor);

    String getName(IRInst* inst);

    void emitDeclarator(IRDeclaratorInfo* declarator);    
    void emitSimpleValue(IRInst* inst) { emitSimpleValueImpl(inst); }
    
    bool shouldFoldInstIntoUseSites(IRInst* inst);

    void emitOperand(IRInst* inst, EmitOpInfo const& outerPrec) { emitOperandImpl(inst, outerPrec); }

    void emitArgs(IRInst* inst);

    
    void emitRateQualifiers(IRInst* value);

    void emitInstResultDecl(IRInst* inst);

    IRTargetSpecificDecoration* findBestTargetDecoration(IRInst* inst);
    IRTargetIntrinsicDecoration* findBestTargetIntrinsicDecoration(IRInst* inst);

    // Check if the string being used to define a target intrinsic
    // is an "ordinary" name, such that we can simply emit a call
    // to the new name with the arguments of the old operation.
    static bool isOrdinaryName(const UnownedStringSlice& name);

    void emitIntrinsicCallExpr(
        IRCall*                         inst,
        IRTargetIntrinsicDecoration*    targetIntrinsic,
        EmitOpInfo const&               inOuterPrec);

    void emitCallExpr(IRCall* inst, EmitOpInfo outerPrec);

    void emitInstExpr(IRInst* inst, EmitOpInfo const& inOuterPrec);
    void defaultEmitInstExpr(IRInst* inst, EmitOpInfo const& inOuterPrec);
    void diagnoseUnhandledInst(IRInst* inst);

    BaseType extractBaseType(IRType* inType);

    void emitInst(IRInst* inst);

    void emitSemantics(IRInst* inst);
    void emitSemanticsUsingVarLayout(IRVarLayout* varLayout);

    static IRVarLayout* getVarLayout(IRInst* var);

    void emitLayoutSemantics(IRInst* inst, char const* uniformSemanticSpelling = "register");

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

    void emitEntryPointAttributes(IRFunc* irFunc, IREntryPointDecoration* entryPointDecor);

    void emitPhiVarDecls(IRFunc* func);

        /// Emit high-level statements for the body of a function.
    void emitFunctionBody(IRGlobalValueWithCode* code);

    void emitSimpleFunc(IRFunc* func) { emitSimpleFuncImpl(func); }

    void emitParamType(IRType* type, String const& name) { emitParamTypeImpl(type, name); }

    IRInst* getSpecializedValue(IRSpecialize* specInst);

    void emitFuncDecl(IRFunc* func);

    IREntryPointLayout* getEntryPointLayout(IRFunc* func);

    IREntryPointLayout* asEntryPoint(IRFunc* func);

        // Detect if the given IR function represents a
        // declaration of an intrinsic/builtin for the
        // current code-generation target.
    bool isTargetIntrinsic(IRFunc* func);

    void emitFunc(IRFunc* func);
    void emitFuncDecorations(IRFunc* func);

    void emitStruct(IRStructType* structType);

    void emitInterpolationModifiers(IRInst* varInst, IRType* valueType, IRVarLayout* layout);

    UInt getRayPayloadLocation(IRInst* inst);

    UInt getCallablePayloadLocation(IRInst* inst);

        /// Emit modifiers that should apply even for a declaration of an SSA temporary.
    virtual void emitTempModifiers(IRInst* temp);

    void emitVarModifiers(IRVarLayout* layout, IRInst* varDecl, IRType* varType);

        /// Emit the array brackets that go on the end of a declaration of the given type.
    void emitArrayBrackets(IRType* inType);

    void emitParameterGroup(IRGlobalParam* varDecl, IRUniformParameterGroupType* type);

    void emitVar(IRVar* varDecl);
    void emitDereferenceOperand(IRInst* inst, EmitOpInfo const& outerPrec);

    void emitGlobalVar(IRGlobalVar* varDecl);
    void emitGlobalParam(IRGlobalParam* varDecl);

    void emitGlobalInst(IRInst* inst);
    virtual void emitGlobalInstImpl(IRInst* inst);

    void ensureInstOperand(ComputeEmitActionsContext* ctx, IRInst* inst, EmitAction::Level requiredLevel = EmitAction::Level::Definition);

    void ensureInstOperandsRec(ComputeEmitActionsContext* ctx, IRInst* inst);

    void ensureGlobalInst(ComputeEmitActionsContext* ctx, IRInst* inst, EmitAction::Level requiredLevel);

    void computeEmitActions(IRModule* module, List<EmitAction>& ioActions);

    void executeEmitActions(List<EmitAction> const& actions);
    void emitModule(IRModule* module, DiagnosticSink* sink)
        { m_irModule = module; emitModuleImpl(module, sink); }

        /// Emit any preprocessor directives that should come *before* the prelude code
        ///
        /// These are directives that are intended to customize some aspect(s) of the
        /// prelude's behavior.
        ///
    void emitPreludeDirectives() { emitPreludeDirectivesImpl(); }

    void emitPreprocessorDirectives() { emitPreprocessorDirectivesImpl(); }
    void emitSimpleType(IRType* type);

    void emitVectorTypeName(IRType* elementType, IRIntegerValue elementCount) { emitVectorTypeNameImpl(elementType, elementCount); }

    virtual RefObject* getExtensionTracker() { return nullptr; }

        /// Gets a source language for a target for a target. Returns Unknown if not a known target
    static SourceLanguage getSourceLanguage(CodeGenTarget target);

        /// Gets the default type name for built in scalar types. Different impls may require something different.
        /// Returns an empty slice if not a built in type
    static UnownedStringSlice getDefaultBuiltinTypeName(IROp op);

        /// Finds the IRNumThreadsDecoration and gets the size from that or sets all dimensions to 1
    static IRNumThreadsDecoration* getComputeThreadGroupSize(IRFunc* func, Int outNumThreads[kThreadGroupAxisCount]);

    protected:

    virtual bool doesTargetSupportPtrTypes() { return false; }
    virtual void emitLayoutSemanticsImpl(IRInst* inst, char const* uniformSemanticSpelling = "register") { SLANG_UNUSED(inst); SLANG_UNUSED(uniformSemanticSpelling); }
    virtual void emitParameterGroupImpl(IRGlobalParam* varDecl, IRUniformParameterGroupType* type) = 0;
    virtual void emitEntryPointAttributesImpl(IRFunc* irFunc, IREntryPointDecoration* entryPointDecor) = 0;

    virtual void emitImageFormatModifierImpl(IRInst* varDecl, IRType* varType) { SLANG_UNUSED(varDecl); SLANG_UNUSED(varType); }
    virtual void emitLayoutQualifiersImpl(IRVarLayout* layout) { SLANG_UNUSED(layout); }
    virtual void emitPreludeDirectivesImpl() {}
    virtual void emitPreprocessorDirectivesImpl() {}
    virtual void emitLayoutDirectivesImpl(TargetRequest* targetReq) { SLANG_UNUSED(targetReq); }
    virtual void emitRateQualifiersImpl(IRRate* rate) { SLANG_UNUSED(rate); }
    virtual void emitSemanticsImpl(IRInst* inst) { SLANG_UNUSED(inst);  }
    virtual void emitSimpleFuncParamImpl(IRParam* param);
    virtual void emitSimpleFuncParamsImpl(IRFunc* func);
    virtual void emitInterpolationModifiersImpl(IRInst* varInst, IRType* valueType, IRVarLayout* layout) { SLANG_UNUSED(varInst); SLANG_UNUSED(valueType); SLANG_UNUSED(layout); }
    virtual void emitSimpleTypeImpl(IRType* type) = 0;
    virtual void emitVarDecorationsImpl(IRInst* varDecl) { SLANG_UNUSED(varDecl);  }
    virtual void emitMatrixLayoutModifiersImpl(IRVarLayout* layout) { SLANG_UNUSED(layout);  }
    virtual void emitTypeImpl(IRType* type, const StringSliceLoc* nameLoc);
    virtual void emitSimpleValueImpl(IRInst* inst);
    virtual void emitModuleImpl(IRModule* module, DiagnosticSink* sink);
    virtual void emitSimpleFuncImpl(IRFunc* func);
    virtual void emitVarExpr(IRInst* inst, EmitOpInfo const& outerPrec);
    virtual void emitOperandImpl(IRInst* inst, EmitOpInfo const& outerPrec);
    virtual void emitParamTypeImpl(IRType* type, String const& name);
    virtual void emitIntrinsicCallExprImpl(IRCall* inst, IRTargetIntrinsicDecoration* targetIntrinsic, EmitOpInfo const& inOuterPrec);
    virtual void emitFunctionPreambleImpl(IRInst* inst) { SLANG_UNUSED(inst); }
    virtual void emitLoopControlDecorationImpl(IRLoopControlDecoration* decl) { SLANG_UNUSED(decl); }
    virtual void emitFuncDecorationImpl(IRDecoration* decoration) { SLANG_UNUSED(decoration); }

        // Only needed for glsl output with $ prefix intrinsics - so perhaps removable in the future
    virtual void emitTextureOrTextureSamplerTypeImpl(IRTextureTypeBase*  type, char const* baseName) { SLANG_UNUSED(type); SLANG_UNUSED(baseName); }
        // Again necessary for & prefix intrinsics. May be removable in the future
    virtual void emitVectorTypeNameImpl(IRType* elementType, IRIntegerValue elementCount) = 0;

    virtual void emitWitnessTable(IRWitnessTable* witnessTable);
    virtual void emitInterface(IRInterfaceType* interfaceType);
    virtual void emitRTTIObject(IRRTTIObject* rttiObject);

    virtual bool tryEmitGlobalParamImpl(IRGlobalParam* varDecl, IRType* varType) { SLANG_UNUSED(varDecl); SLANG_UNUSED(varType); return false; }
    virtual bool tryEmitInstExprImpl(IRInst* inst, const EmitOpInfo& inOuterPrec) { SLANG_UNUSED(inst); SLANG_UNUSED(inOuterPrec); return false; }

        /// Inspect the capabilities required by `inst` (according to its decorations),
        /// and ensure that those capabilities have been detected and stored in the
        /// target-specific extension tracker.
    void handleRequiredCapabilities(IRInst* inst);
    virtual void handleRequiredCapabilitiesImpl(IRInst* inst) { SLANG_UNUSED(inst); }

    void _emitArrayType(IRArrayType* arrayType, EDeclarator* declarator);
    void _emitUnsizedArrayType(IRUnsizedArrayType* arrayType, EDeclarator* declarator);
    void _emitType(IRType* type, EDeclarator* declarator);
    void _emitInst(IRInst* inst);

        // Emit the argument list (including paranthesis) in a `CallInst`
    void _emitCallArgList(IRCall* call);


    String _generateUniqueName(const UnownedStringSlice& slice);

        // Sort witnessTable entries according to the order defined in the witnessed interface type.
    List<IRWitnessTableEntry*> getSortedWitnessTableEntries(IRWitnessTable* witnessTable);
    
    BackEndCompileRequest* m_compileRequest = nullptr;
    IRModule* m_irModule = nullptr;

    // The stage for which we are emitting code.
    //
    // TODO: We should support emitting code that includes multiple
    // entry points for different stages, but this value is used
    // in some very specific cases to determine how a construct
    // should map to GLSL.
    //
    Stage m_entryPointStage = Stage::Unknown;

    // The target language we want to generate code for
    CodeGenTarget m_target;

        /// The capabilities of the target
    CapabilitySet m_targetCaps;

    // Source language (based on the more nuanced m_target)
    SourceLanguage m_sourceLanguage;

    // Where source is written to
    SourceWriter* m_writer;

    UInt m_uniqueIDCounter = 1;
    Dictionary<IRInst*, UInt> m_mapIRValueToID;

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
