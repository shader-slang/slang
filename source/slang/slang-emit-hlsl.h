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

    HLSLSourceEmitter(const Desc& desc) :
        Super(desc)
    {}

protected:

    virtual void emitLayoutSemanticsImpl(IRInst* inst, char const* uniformSemanticSpelling) SLANG_OVERRIDE;
    virtual void emitParameterGroupImpl(IRGlobalParam* varDecl, IRUniformParameterGroupType* type) SLANG_OVERRIDE;
    virtual void emitEntryPointAttributesImpl(IRFunc* irFunc, EntryPointLayout* entryPointLayout) SLANG_OVERRIDE;
    virtual void emitLayoutDirectivesImpl(TargetRequest* targetReq) SLANG_OVERRIDE;
    virtual void emitRateQualifiersImpl(IRRate* rate) SLANG_OVERRIDE;
    virtual void emitSemanticsImpl(IRInst* inst) SLANG_OVERRIDE;
    virtual void emitSimpleFuncParamImpl(IRParam* param) SLANG_OVERRIDE;
    virtual void emitInterpolationModifiersImpl(IRInst* varInst, IRType* valueType, VarLayout* layout) SLANG_OVERRIDE;
    virtual void emitSimpleTypeImpl(IRType* type) SLANG_OVERRIDE;
    virtual void emitVectorTypeNameImpl(IRType* elementType, IRIntegerValue elementCount) SLANG_OVERRIDE;
    virtual void emitVarDecorationsImpl(IRInst* varDecl) SLANG_OVERRIDE;
    virtual void emitMatrixLayoutModifiersImpl(VarLayout* layout) SLANG_OVERRIDE;

    virtual bool tryEmitInstExprImpl(IRInst* inst, const EmitOpInfo& inOuterPrec) SLANG_OVERRIDE;

        // Emit a single `register` semantic, as appropriate for a given resource-type-specific layout info
        // Keyword to use in the uniform case (`register` for globals, `packoffset` inside a `cbuffer`)
    void _emitHLSLRegisterSemantic(LayoutResourceKind kind, EmitVarChain* chain, char const* uniformSemanticSpelling = "register");

        // Emit all the `register` semantics that are appropriate for a particular variable layout
    void _emitHLSLRegisterSemantics(EmitVarChain* chain, char const* uniformSemanticSpelling = "register");
    void _emitHLSLRegisterSemantics(VarLayout* varLayout, char const* uniformSemanticSpelling = "register");

    void _emitHLSLParameterGroupFieldLayoutSemantics(EmitVarChain* chain);
    void _emitHLSLParameterGroupFieldLayoutSemantics(RefPtr<VarLayout> fieldLayout, EmitVarChain* inChain);

    void _emitHLSLParameterGroup(IRGlobalParam* varDecl, IRUniformParameterGroupType* type);

    void _emitHLSLEntryPointAttributes(IRFunc* irFunc, EntryPointLayout* entryPointLayout);

    void _emitHLSLTextureType(IRTextureTypeBase* texType);

    void _emitHLSLFuncDeclPatchConstantFuncAttribute(IRFunc* irFunc, FuncDecl* entryPoint, PatchConstantFuncAttribute* attrib);

    void _emitHLSLAttributeSingleString(const char* name, FuncDecl* entryPoint, Attribute* attrib);

    void _emitHLSLAttributeSingleInt(const char* name, FuncDecl* entryPoint, Attribute* attrib);

};

}
#endif
