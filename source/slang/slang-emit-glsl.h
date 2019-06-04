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

    void emitGLSLTextureOrTextureSamplerType(IRTextureTypeBase*  type, char const* baseName);
    void emitGLSLTextureSamplerType(IRTextureSamplerType* type);
    void emitGLSLImageType(IRGLSLImageType* type);

    void emitGLSLTextureType(IRTextureType* texType);

    void emitIRStructuredBuffer_GLSL(IRGlobalParam* varDecl, IRHLSLStructuredBufferTypeBase* structuredBufferType);

    void emitIRByteAddressBuffer_GLSL(IRGlobalParam* varDecl, IRByteAddressBufferTypeBase* byteAddressBufferType);
    void emitGLSLParameterGroup(IRGlobalParam* varDecl, IRUniformParameterGroupType* type);

    void emitGLSLImageFormatModifier(IRInst* var, IRTextureType* resourceType);

    void emitGLSLLayoutQualifiers(RefPtr<VarLayout> layout, EmitVarChain* inChain, LayoutResourceKind filter = LayoutResourceKind::None);
    bool emitGLSLLayoutQualifier(LayoutResourceKind kind, EmitVarChain* chain);

    void emitGLSLTypePrefix(IRType* type, bool promoteHalfToFloat = false);

    GLSLSourceEmitter(const Desc& desc) :
        Super(desc)
    {
    }

protected:

    virtual void emitIRParameterGroupImpl(IRGlobalParam* varDecl, IRUniformParameterGroupType* type) SLANG_OVERRIDE;
    virtual void emitIREntryPointAttributesImpl(IRFunc* irFunc, EntryPointLayout* entryPointLayout) SLANG_OVERRIDE;
    virtual void emitTextureTypeImpl(IRTextureType* texType) SLANG_OVERRIDE;
    virtual void emitImageTypeImpl(IRGLSLImageType* type) SLANG_OVERRIDE;
    virtual void emitImageFormatModifierImpl(IRInst* varDecl, IRType* varType) SLANG_OVERRIDE;
    virtual void emitLayoutQualifiersImpl(VarLayout* layout) SLANG_OVERRIDE;
    virtual void emitTextureSamplerTypeImpl(IRTextureSamplerType* type) SLANG_OVERRIDE;
    virtual void emitVectorTypeNameImpl(IRType* elementType, IRIntegerValue elementCount) SLANG_OVERRIDE;
    virtual void emitMatrixTypeImpl(IRMatrixType* matType) SLANG_OVERRIDE;

    virtual void emitTextureOrTextureSamplerTypeImpl(IRTextureTypeBase*  type, char const* baseName) SLANG_OVERRIDE { emitGLSLTextureOrTextureSamplerType(type, baseName); }

    virtual bool tryEmitIRGlobalParamImpl(IRGlobalParam* varDecl, IRType* varType) SLANG_OVERRIDE;

};

}
#endif
