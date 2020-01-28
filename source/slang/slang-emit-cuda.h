// slang-emit-cuda.h
#ifndef SLANG_EMIT_CUDA_H
#define SLANG_EMIT_CUDA_H

#include "slang-emit-cpp.h"

namespace Slang
{

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

    static UnownedStringSlice getBuiltinTypeName(IROp op);
    static UnownedStringSlice getVectorPrefix(IROp op);

    CUDASourceEmitter(const Desc& desc) :
        Super(desc)
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
    virtual void emitCall(const HLSLIntrinsic* specOp, IRInst* inst, const IRUse* operands, int numOperands, const EmitOpInfo& inOuterPrec) SLANG_OVERRIDE;
    virtual void emitFunctionPreambleImpl(IRInst* inst) SLANG_OVERRIDE { SLANG_UNUSED(inst); m_writer->emit("__device__ "); }


    //virtual bool tryEmitGlobalParamImpl(IRGlobalParam* varDecl, IRType* varType) SLANG_OVERRIDE;
    virtual bool tryEmitInstExprImpl(IRInst* inst, const EmitOpInfo& inOuterPrec) SLANG_OVERRIDE;

    virtual void emitPreprocessorDirectivesImpl() SLANG_OVERRIDE;

    virtual void emitModuleImpl(IRModule* module) SLANG_OVERRIDE;

    // CPPSourceEmitter overrides 
    virtual SlangResult calcTypeName(IRType* type, CodeGenTarget target, StringBuilder& out) SLANG_OVERRIDE;
    virtual SlangResult calcScalarFuncName(HLSLIntrinsic::Op op, IRBasicType* type, StringBuilder& outBuilder) SLANG_OVERRIDE;
    
    SlangResult _calcCUDATextureTypeName(IRTextureTypeBase* texType, StringBuilder& outName);
};

}
#endif
