// slang-emit-cuda.h
#ifndef SLANG_EMIT_CUDA_H
#define SLANG_EMIT_CUDA_H

#include "slang-emit-cpp.h"

namespace Slang
{

class CUDAExtensionTracker : public RefObject
{
public:

    typedef uint32_t BaseTypeFlags;

    SemanticVersion m_smVersion;

    void requireBaseType(BaseType baseType) { m_baseTypeFlags |= _getFlag(baseType); }
    bool isBaseTypeRequired(BaseType baseType) { return (m_baseTypeFlags & _getFlag(baseType)) != 0; }

        /// Ensure that the generated code is compiled for at least CUDA SM `version`
    void requireSMVersion(const SemanticVersion& smVersion) { m_smVersion = (smVersion > m_smVersion) ? smVersion : m_smVersion; }

        /// Should be called before reading out values. 
    void finalize();

protected:

    static BaseTypeFlags _getFlag(BaseType baseType) { return BaseTypeFlags(1) << int(baseType); }

    BaseTypeFlags m_baseTypeFlags = 0; 
};

class CUDASourceEmitter : public CPPSourceEmitter
{
public:
    typedef CPPSourceEmitter Super;

    typedef uint32_t SemanticUsedFlags;
    struct SemanticUsedFlag
    {
        enum Enum : SemanticUsedFlags
        {
            DispatchThreadID = 0x01,
            GroupThreadID = 0x02,
            GroupID = 0x04,
        };
    };

    UnownedStringSlice getBuiltinTypeName(IROp op);
    UnownedStringSlice getVectorPrefix(IROp op);

    virtual RefObject* getExtensionTracker() SLANG_OVERRIDE { return m_extensionTracker; }
    virtual void emitTempModifiers(IRInst* temp) SLANG_OVERRIDE;

    CUDASourceEmitter(const Desc& desc) :
        Super(desc),
        m_extensionTracker(new CUDAExtensionTracker)
    {}

protected:

    virtual void emitLayoutSemanticsImpl(IRInst* inst, char const* uniformSemanticSpelling) SLANG_OVERRIDE;
    virtual void emitParameterGroupImpl(IRGlobalParam* varDecl, IRUniformParameterGroupType* type) SLANG_OVERRIDE;
    virtual void emitEntryPointAttributesImpl(IRFunc* irFunc, IREntryPointDecoration* entryPointDecor) SLANG_OVERRIDE;
    virtual void emitLayoutDirectivesImpl(TargetRequest* targetReq) SLANG_OVERRIDE;
    virtual void emitRateQualifiersImpl(IRRate* rate) SLANG_OVERRIDE;
    virtual void emitSemanticsImpl(IRInst* inst) SLANG_OVERRIDE;
    virtual void emitSimpleFuncImpl(IRFunc* func) SLANG_OVERRIDE;
    virtual void emitSimpleFuncParamsImpl(IRFunc* func) SLANG_OVERRIDE;
    virtual void emitInterpolationModifiersImpl(IRInst* varInst, IRType* valueType, IRVarLayout* layout) SLANG_OVERRIDE;
    virtual void emitSimpleTypeImpl(IRType* type) SLANG_OVERRIDE;
    virtual void emitVectorTypeNameImpl(IRType* elementType, IRIntegerValue elementCount) SLANG_OVERRIDE;
    virtual void emitVarDecorationsImpl(IRInst* varDecl) SLANG_OVERRIDE;
    virtual void emitMatrixLayoutModifiersImpl(IRVarLayout* layout) SLANG_OVERRIDE;
    virtual void emitCall(const HLSLIntrinsic* specOp, IRInst* inst, const IRUse* operands, int numOperands, const EmitOpInfo& inOuterPrec) SLANG_OVERRIDE;
    virtual void emitFunctionPreambleImpl(IRInst* inst) SLANG_OVERRIDE;
    virtual String generateEntryPointNameImpl(IREntryPointDecoration* entryPointDecor) SLANG_OVERRIDE;

    virtual void emitGlobalRTTISymbolPrefix() SLANG_OVERRIDE;

    virtual void emitLoopControlDecorationImpl(IRLoopControlDecoration* decl) SLANG_OVERRIDE;

    virtual void handleRequiredCapabilitiesImpl(IRInst* inst) SLANG_OVERRIDE;

    virtual bool tryEmitGlobalParamImpl(IRGlobalParam* varDecl, IRType* varType) SLANG_OVERRIDE;
    virtual bool tryEmitInstExprImpl(IRInst* inst, const EmitOpInfo& inOuterPrec) SLANG_OVERRIDE;

    virtual void emitPreprocessorDirectivesImpl() SLANG_OVERRIDE;

    virtual void emitModuleImpl(IRModule* module, DiagnosticSink* sink) SLANG_OVERRIDE;

    // CPPSourceEmitter overrides 
    virtual SlangResult calcTypeName(IRType* type, CodeGenTarget target, StringBuilder& out) SLANG_OVERRIDE;
    virtual SlangResult calcScalarFuncName(HLSLIntrinsic::Op op, IRBasicType* type, StringBuilder& outBuilder) SLANG_OVERRIDE;

    virtual void emitSpecializedOperationDefinition(const HLSLIntrinsic* specOp) SLANG_OVERRIDE;

    SlangResult _calcCUDATextureTypeName(IRTextureTypeBase* texType, StringBuilder& outName);

    void _emitInitializerList(IRType* elementType, IRUse* operands, Index operandCount);
    void _emitInitializerListValue(IRType* elementType, IRInst* value);

    void _emitGetHalfVectorElement(IRInst* baseInst, Index index, Index vecSize, const EmitOpInfo& inOuterPrec);
    
    RefPtr<CUDAExtensionTracker> m_extensionTracker;
};

}
#endif
