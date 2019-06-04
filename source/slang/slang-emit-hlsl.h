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

    void emitIREntryPointAttributes_HLSL(IRFunc* irFunc, EntryPointLayout* entryPointLayout);
   
    HLSLSourceEmitter(const Desc& desc) :
        Super(desc)
    {}

protected:

    virtual void emitIRLayoutSemanticsImpl(IRInst* inst, char const* uniformSemanticSpelling) SLANG_OVERRIDE;
    virtual void emitIRParameterGroupImpl(IRGlobalParam* varDecl, IRUniformParameterGroupType* type) SLANG_OVERRIDE;
    virtual void emitIREntryPointAttributesImpl(IRFunc* irFunc, EntryPointLayout* entryPointLayout) SLANG_OVERRIDE;



};

}
#endif
