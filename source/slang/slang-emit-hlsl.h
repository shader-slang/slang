// slang-emit-hlsl.h
#ifndef SLANG_EMIT_HLSL_H
#define SLANG_EMIT_HLSL_H

#include "slang-emit-c-like.h"

namespace Slang
{

class HLSLExtensionTracker : public RefObject
{
public:
        /// Has any operation been used that requires NVAPI to be included via prelude?
    bool m_requiresNVAPI = false;
};

class HLSLSourceEmitter : public CLikeSourceEmitter
{
public:
    typedef CLikeSourceEmitter Super;

    HLSLSourceEmitter(const Desc& desc)
        : Super(desc)
        , m_extensionTracker(new HLSLExtensionTracker)
    {}

    virtual RefObject* getExtensionTracker() SLANG_OVERRIDE { return m_extensionTracker; }

protected:
    RefPtr<HLSLExtensionTracker> m_extensionTracker;

    virtual void emitLayoutSemanticsImpl(IRInst* inst, char const* uniformSemanticSpelling) SLANG_OVERRIDE;
    virtual void emitParameterGroupImpl(IRGlobalParam* varDecl, IRUniformParameterGroupType* type) SLANG_OVERRIDE;
    virtual void emitEntryPointAttributesImpl(IRFunc* irFunc, IREntryPointDecoration* entryPointDecor) SLANG_OVERRIDE;
    virtual void emitLayoutDirectivesImpl(TargetRequest* targetReq) SLANG_OVERRIDE;
    virtual void emitRateQualifiersImpl(IRRate* rate) SLANG_OVERRIDE;
    virtual void emitSemanticsImpl(IRInst* inst) SLANG_OVERRIDE;
    virtual void emitSimpleFuncParamImpl(IRParam* param) SLANG_OVERRIDE;
    virtual void emitInterpolationModifiersImpl(IRInst* varInst, IRType* valueType, IRVarLayout* layout) SLANG_OVERRIDE;
    virtual void emitSimpleTypeImpl(IRType* type) SLANG_OVERRIDE;
    virtual void emitVectorTypeNameImpl(IRType* elementType, IRIntegerValue elementCount) SLANG_OVERRIDE;
    virtual void emitVarDecorationsImpl(IRInst* varDecl) SLANG_OVERRIDE;
    virtual void emitMatrixLayoutModifiersImpl(IRVarLayout* layout) SLANG_OVERRIDE;

    virtual bool tryEmitInstExprImpl(IRInst* inst, const EmitOpInfo& inOuterPrec) SLANG_OVERRIDE;
    virtual void emitSimpleValueImpl(IRInst* inst) SLANG_OVERRIDE;
    virtual void emitLoopControlDecorationImpl(IRLoopControlDecoration* decl) SLANG_OVERRIDE;
    virtual void emitFuncDecorationImpl(IRDecoration* decoration) SLANG_OVERRIDE;


    virtual void handleRequiredCapabilitiesImpl(IRInst* inst) SLANG_OVERRIDE;
    virtual void emitPreludeDirectivesImpl() SLANG_OVERRIDE;

    virtual void emitGlobalInstImpl(IRInst* inst) SLANG_OVERRIDE;

        // Emit a single `register` semantic, as appropriate for a given resource-type-specific layout info
        // Keyword to use in the uniform case (`register` for globals, `packoffset` inside a `cbuffer`)
    void _emitHLSLRegisterSemantic(LayoutResourceKind kind, EmitVarChain* chain, char const* uniformSemanticSpelling = "register");

        // Emit all the `register` semantics that are appropriate for a particular variable layout
    void _emitHLSLRegisterSemantics(EmitVarChain* chain, char const* uniformSemanticSpelling = "register");
    void _emitHLSLRegisterSemantics(IRVarLayout* varLayout, char const* uniformSemanticSpelling = "register");

    void _emitHLSLParameterGroupFieldLayoutSemantics(EmitVarChain* chain);
    void _emitHLSLParameterGroupFieldLayoutSemantics(IRVarLayout* fieldLayout, EmitVarChain* inChain);

    void _emitHLSLParameterGroup(IRGlobalParam* varDecl, IRUniformParameterGroupType* type);

    void _emitHLSLTextureType(IRTextureTypeBase* texType);

    void _emitHLSLDecorationSingleString(const char* name, IRFunc* entryPoint, IRStringLit* val);
    void _emitHLSLDecorationSingleInt(const char* name, IRFunc* entryPoint, IRIntLit* val);
    
};

}
#endif
