// slang-emit-glsl.h
#ifndef SLANG_EMIT_GLSL_H
#define SLANG_EMIT_GLSL_H

#include "slang-emit-c-like.h"

namespace Slang
{

class GLSLSourceEmitter : public CLikeSourceEmitter
{
public:
    typedef CLikeSourceEmitter Super;

    void emitGLSLTextureOrTextureSamplerType(IRTextureTypeBase* type, char const* baseName);
    
    void emitIRStructuredBuffer_GLSL(IRGlobalParam* varDecl, IRHLSLStructuredBufferTypeBase* structuredBufferType);

    void emitIRByteAddressBuffer_GLSL(IRGlobalParam* varDecl, IRByteAddressBufferTypeBase* byteAddressBufferType);
    void emitGLSLParameterGroup(IRGlobalParam* varDecl, IRUniformParameterGroupType* type);

    void emitGLSLImageFormatModifier(IRInst* var, IRTextureType* resourceType);

    void emitGLSLLayoutQualifiers(RefPtr<VarLayout> layout, EmitVarChain* inChain, LayoutResourceKind filter = LayoutResourceKind::None);
    bool emitGLSLLayoutQualifier(LayoutResourceKind kind, EmitVarChain* chain);

    void emitGLSLTypePrefix(IRType* type, bool promoteHalfToFloat = false);

    void requireGLSLExtension(const String& name);

    void requireGLSLVersion(ProfileVersion version);
    void requireGLSLVersion(int version);

        // Emit the `flat` qualifier if the underlying type
        // of the variable is an integer type.
    void maybeEmitGLSLFlatModifier(IRType* valueType);

    GLSLSourceEmitter(const Desc& desc) :
        Super(desc)
    {
    }

protected:

    virtual void emitParameterGroupImpl(IRGlobalParam* varDecl, IRUniformParameterGroupType* type) SLANG_OVERRIDE;
    virtual void emitEntryPointAttributesImpl(IRFunc* irFunc, EntryPointLayout* entryPointLayout) SLANG_OVERRIDE;
    virtual void emitTextureTypeImpl(IRTextureType* texType) SLANG_OVERRIDE;
    virtual void emitImageTypeImpl(IRGLSLImageType* type) SLANG_OVERRIDE;
    virtual void emitImageFormatModifierImpl(IRInst* varDecl, IRType* varType) SLANG_OVERRIDE;
    virtual void emitLayoutQualifiersImpl(VarLayout* layout) SLANG_OVERRIDE;
    virtual void emitTextureSamplerTypeImpl(IRTextureSamplerType* type) SLANG_OVERRIDE;
    virtual void emitVectorTypeNameImpl(IRType* elementType, IRIntegerValue elementCount) SLANG_OVERRIDE;
    virtual void emitMatrixTypeImpl(IRMatrixType* matType) SLANG_OVERRIDE;

    virtual void emitTextureOrTextureSamplerTypeImpl(IRTextureTypeBase*  type, char const* baseName) SLANG_OVERRIDE { emitGLSLTextureOrTextureSamplerType(type, baseName); }
    virtual void emitUntypedBufferTypeImpl(IRUntypedBufferResourceType* type) SLANG_OVERRIDE;
    virtual void emitStructuredBufferTypeImpl(IRHLSLStructuredBufferTypeBase* type) SLANG_OVERRIDE;
    virtual void emitSamplerStateTypeImpl(IRSamplerStateTypeBase* samplerStateType) SLANG_OVERRIDE;

    virtual void emitPreprocessorDirectivesImpl() SLANG_OVERRIDE;
    virtual void emitLayoutDirectivesImpl(TargetRequest* targetReq) SLANG_OVERRIDE;
    virtual void emitRateQualifiersImpl(IRRate* rate) SLANG_OVERRIDE;
    virtual void emitInterpolationModifiersImpl(IRInst* varInst, IRType* valueType, VarLayout* layout) SLANG_OVERRIDE;

    virtual void handleCallExprDecorationsImpl(IRInst* funcValue) SLANG_OVERRIDE;

    virtual bool tryEmitGlobalParamImpl(IRGlobalParam* varDecl, IRType* varType) SLANG_OVERRIDE;
    virtual bool tryEmitInstExprImpl(IRInst* inst, IREmitMode mode, const EmitOpInfo& inOuterPrec) SLANG_OVERRIDE;
    virtual bool tryEmitSimpleTypeImpl(IRType* type) SLANG_OVERRIDE;

    void _requireHalf();
    void _maybeEmitGLSLCast(IRType* castType, IRInst* inst, IREmitMode mode);
};

}
#endif
