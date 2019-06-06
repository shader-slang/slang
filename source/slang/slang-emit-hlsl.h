// slang-emit-hlsl.h
#ifndef SLANG_EMIT_HLSL_H
#define SLANG_EMIT_HLSL_H

#include "slang-emit-c-like.h"

namespace Slang
{

class HLSLSourceEmitter : public CLikeSourceEmitter
{
public:
    typedef CLikeSourceEmitter Super;

        // Emit a single `register` semantic, as appropriate for a given resource-type-specific layout info
        // Keyword to use in the uniform case (`register` for globals, `packoffset` inside a `cbuffer`)
    void emitHLSLRegisterSemantic(LayoutResourceKind kind, EmitVarChain* chain, char const* uniformSemanticSpelling = "register");

        // Emit all the `register` semantics that are appropriate for a particular variable layout
    void emitHLSLRegisterSemantics(EmitVarChain* chain, char const* uniformSemanticSpelling = "register");
    void emitHLSLRegisterSemantics(VarLayout* varLayout, char const* uniformSemanticSpelling = "register");

    void emitHLSLParameterGroupFieldLayoutSemantics(EmitVarChain* chain);

    void emitHLSLParameterGroupFieldLayoutSemantics(RefPtr<VarLayout> fieldLayout, EmitVarChain* inChain);

    void emitHLSLParameterGroup(IRGlobalParam* varDecl, IRUniformParameterGroupType* type);

    void emitHLSLEntryPointAttributes(IRFunc* irFunc, EntryPointLayout* entryPointLayout);

    void emitHLSLTextureType(IRTextureTypeBase* texType);

    void emitHLSLFuncDeclPatchConstantFuncAttribute(IRFunc* irFunc, FuncDecl* entryPoint, PatchConstantFuncAttribute* attrib);

    void emitHLSLAttributeSingleString(const char* name, FuncDecl* entryPoint, Attribute* attrib);

    void emitHLSLAttributeSingleInt(const char* name, FuncDecl* entryPoint, Attribute* attrib);

    HLSLSourceEmitter(const Desc& desc) :
        Super(desc)
    {}

protected:

    virtual void emitIRLayoutSemanticsImpl(IRInst* inst, char const* uniformSemanticSpelling) SLANG_OVERRIDE;
    virtual void emitIRParameterGroupImpl(IRGlobalParam* varDecl, IRUniformParameterGroupType* type) SLANG_OVERRIDE;
    virtual void emitIREntryPointAttributesImpl(IRFunc* irFunc, EntryPointLayout* entryPointLayout) SLANG_OVERRIDE;
    virtual void emitTextureTypeImpl(IRTextureType* texType) SLANG_OVERRIDE;
    virtual void emitImageTypeImpl(IRGLSLImageType* type) SLANG_OVERRIDE;
    virtual void emitVectorTypeNameImpl(IRType* elementType, IRIntegerValue elementCount) SLANG_OVERRIDE;
    virtual void emitMatrixTypeImpl(IRMatrixType* matType) SLANG_OVERRIDE;
    virtual void emitUntypedBufferTypeImpl(IRUntypedBufferResourceType* type) SLANG_OVERRIDE;
    virtual void emitStructuredBufferTypeImpl(IRHLSLStructuredBufferTypeBase* type) SLANG_OVERRIDE;
    virtual void emitSamplerStateTypeImpl(IRSamplerStateTypeBase* samplerStateType) SLANG_OVERRIDE;
    virtual void emitLayoutDirectivesImpl(TargetRequest* targetReq) SLANG_OVERRIDE;
    virtual void emitRateQualifiersImpl(IRRate* rate) SLANG_OVERRIDE;
    virtual void emitIRSemanticsImpl(IRInst* inst) SLANG_OVERRIDE;
    virtual void emitSimpleFuncParamImpl(IRParam* param) SLANG_OVERRIDE;

    virtual bool tryEmitIRInstExprImpl(IRInst* inst, IREmitMode mode, const EmitOpInfo& inOuterPrec) SLANG_OVERRIDE;
    virtual bool tryEmitSimpleTypeImpl(IRType* type) SLANG_OVERRIDE;
};

}
#endif
