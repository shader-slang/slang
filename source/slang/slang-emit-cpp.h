// slang-emit-cpp.h
#ifndef SLANG_EMIT_CPP_H
#define SLANG_EMIT_CPP_H

#include "slang-emit-c-like.h"
#include "slang-ir-clone.h"

#include "../core/slang-string-slice-pool.h"

namespace Slang
{

class CPPSourceEmitter: public CLikeSourceEmitter
{
public:
    typedef CLikeSourceEmitter Super;

    struct HLSLFunction
    {
        typedef HLSLFunction ThisType;

        UInt GetHashCode() const { return combineHash(int(name), Slang::GetHashCode(signatureType)); }

        bool operator==(const ThisType& rhs) const { return name == rhs.name && returnType == rhs.returnType && signatureType == rhs.signatureType; }
        bool operator!=(const ThisType& rhs) const { return !(*this == rhs); }

        bool isScalar() const
        {
            int paramCount = int(signatureType->getParamCount());
            for (int i = 0; i < paramCount; ++i)
            {
                IRType* paramType = signatureType->getParamType(i);
                // If any are vec or matrix, then we
                if (paramType->op == kIROp_MatrixType || paramType->op == kIROp_VectorType)
                {
                    return false;
                }
            }
            return true;
        }

        StringSlicePool::Handle name;
        IRType* returnType;
        IRFuncType* signatureType;              // Same as funcType, but has return type of void
    };

    enum class BuiltInCOp
    {
        Splat,                  //< Splat a single value to all values of a vector or matrix type
        Init,                   //< Initialize with parameters (must match the type)
    };

    CPPSourceEmitter(const Desc& desc);

    static UnownedStringSlice getBuiltinTypeName(IROp op);

protected:
    typedef SlangResult (*EmitFunc)(const HLSLFunction& func, CPPSourceEmitter* emitter);
    
    void _emitCFunc(BuiltInCOp cop, IRType* type);

    virtual void emitParameterGroupImpl(IRGlobalParam* varDecl, IRUniformParameterGroupType* type) SLANG_OVERRIDE;
    virtual void emitEntryPointAttributesImpl(IRFunc* irFunc, EntryPointLayout* entryPointLayout) SLANG_OVERRIDE;
    virtual void emitSimpleTypeImpl(IRType* type) SLANG_OVERRIDE;
    virtual void emitVectorTypeNameImpl(IRType* elementType, IRIntegerValue elementCount) SLANG_OVERRIDE;

    virtual bool tryEmitInstExprImpl(IRInst* inst, IREmitMode mode, const EmitOpInfo& inOuterPrec) SLANG_OVERRIDE;

    virtual void emitPreprocessorDirectivesImpl();

    void emitIntrinsicCallExpr(IRCall* inst, IRFunc* func, IREmitMode mode, EmitOpInfo const& inOuterPrec);

    IRInst* _clone(IRInst* inst);
    IRType* _cloneType(IRType* type) { return (IRType*)_clone((IRInst*)type); }

    HLSLFunction _getHLSLFunc(const UnownedStringSlice& name, IRCall* inst, int operandIndex, int operandCount);
    HLSLFunction _getHLSLFunc(const UnownedStringSlice& name, IRInst* inst);

    UnownedStringSlice _getFuncName(const HLSLFunction& func);
    StringSlicePool::Handle _calcFuncName(const HLSLFunction& func);

    IRType* _getVecType(IRType* elementType, int count);

    UnownedStringSlice _getTypeName(IRType* type);
    StringSlicePool::Handle _calcTypeName(IRType* type);
    
    Dictionary<IRType*, StringSlicePool::Handle> m_typeNameMap;
    Dictionary<HLSLFunction, StringSlicePool::Handle> m_funcNameMap;

    RefPtr<IRModule> m_uniqueModule;            ///< Store types/function sigs etc for output
    SharedIRBuilder m_sharedIRBuilder;
    IRBuilder m_irBuilder;

    Dictionary<IRInst*, IRInst*> m_cloneMap;

    IRCloneEnv m_cloneEnv;

    StringSlicePool m_slicePool;
};

}
#endif
