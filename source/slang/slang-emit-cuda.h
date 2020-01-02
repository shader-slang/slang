// slang-emit-cuda.h
#ifndef SLANG_EMIT_CUDA_H
#define SLANG_EMIT_CUDA_H

#include "slang-emit-c-like.h"

namespace Slang
{

class CUDASourceEmitter : public CLikeSourceEmitter
{
public:
    typedef CLikeSourceEmitter Super;

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

    static UnownedStringSlice getBuiltinTypeName(IROp op);
    static UnownedStringSlice getVectorPrefix(IROp op);

    CUDASourceEmitter(const Desc& desc) :
        Super(desc),
        m_slicePool(StringSlicePool::Style::Default)
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
    virtual void emitOperandImpl(IRInst* inst, EmitOpInfo const&  outerPrec) SLANG_OVERRIDE;

    virtual bool tryEmitInstExprImpl(IRInst* inst, const EmitOpInfo& inOuterPrec) SLANG_OVERRIDE;

        // Emit a single `register` semantic, as appropriate for a given resource-type-specific layout info
        // Keyword to use in the uniform case (`register` for globals, `packoffset` inside a `cbuffer`)
    void _emitCUDARegisterSemantic(LayoutResourceKind kind, EmitVarChain* chain, char const* uniformSemanticSpelling = "register");

        // Emit all the `register` semantics that are appropriate for a particular variable layout
    void _emitCUDARegisterSemantics(EmitVarChain* chain, char const* uniformSemanticSpelling = "register");
    void _emitCUDARegisterSemantics(IRVarLayout* varLayout, char const* uniformSemanticSpelling = "register");

    void _emitCUDAParameterGroupFieldLayoutSemantics(EmitVarChain* chain);
    void _emitCUDAParameterGroupFieldLayoutSemantics(IRVarLayout* fieldLayout, EmitVarChain* inChain);

    void _emitCUDAParameterGroup(IRGlobalParam* varDecl, IRUniformParameterGroupType* type);

    void _emitCUDADecorationSingleString(const char* name, IRFunc* entryPoint, IRStringLit* val);
    void _emitCUDADecorationSingleInt(const char* name, IRFunc* entryPoint, IRIntLit* val);

    SlangResult _calcCUDATypeName(IRType* type, StringBuilder& out);
    UnownedStringSlice _getCUDATypeName(IRType* inType);
    SlangResult _calcCUDATextureTypeName(IRTextureTypeBase* texType, StringBuilder& outName);



    Dictionary<IRType*, StringSlicePool::Handle> m_typeNameMap;
    StringSlicePool m_slicePool;

    UInt m_semanticUsedFlags = 0;
};

}
#endif
