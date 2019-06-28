// slang-emit-cpp.h
#ifndef SLANG_EMIT_CPP_H
#define SLANG_EMIT_CPP_H

#include "slang-emit-c-like.h"
#include "slang-ir-clone.h"

#include "../core/slang-string-slice-pool.h"

namespace Slang
{

class CPPSourceEmitter;

#define SLANG_CPP_OPERATION(x) \
        x(Invalid) \
        x(Init) \
        x(Broadcast) \
        x(Splat) \
        x(Mul) \
        x(Div) \
        x(Add) \
        x(Sub) \
        \
        x(Dot) \
        \
        x(VecMatMul)

class CPPEmitHandler: public RefObject
{
public:
#define SLANG_CPP_OPERATION_ENUM(x) x,

    enum class Operation
    {
        SLANG_CPP_OPERATION(SLANG_CPP_OPERATION_ENUM)
    };

    struct SpecializedOperation
    {
        typedef SpecializedOperation ThisType;

        UInt GetHashCode() const { return combineHash(int(op), Slang::GetHashCode(signatureType)); }

        bool operator==(const ThisType& rhs) const { return op == rhs.op && returnType == rhs.returnType && signatureType == rhs.signatureType; }
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

        Operation op;
        IRType* returnType;
        IRFuncType* signatureType;              // Same as funcType, but has return type of void
    };

    virtual SpecializedOperation getSpecializedOperation(Operation op, IRType*const* argTypes, int argTypesCount, IRType* retType);
    virtual void useType(IRType* type);
    virtual void emitCall(const SpecializedOperation& specOp, const IRUse* operands, int numOperands, CLikeSourceEmitter::IREmitMode mode, const EmitOpInfo& inOuterPrec, CPPSourceEmitter* emitter);
    virtual void emitType(IRType* type, CPPSourceEmitter* emitter);
    virtual void emitTypeDefinition(IRType* type, CPPSourceEmitter* emitter);
    virtual void emitSpecializedOperationDefinition(const SpecializedOperation& specOp, CPPSourceEmitter* emitter);
    virtual void emitVectorTypeName(IRType* elementType, int elementCount, CPPSourceEmitter* emitter);

    virtual void emitPreamble(CPPSourceEmitter* emitter);

    void emitOperationCall(Operation op, IRUse* operands, int operandCount, IRType* retType, CLikeSourceEmitter::IREmitMode mode, const EmitOpInfo& inOuterPrec, CPPSourceEmitter* emitter);

    static UnownedStringSlice getBuiltinTypeName(IROp op);
    static UnownedStringSlice getName(Operation op);

    Operation getOperationByName(const UnownedStringSlice& slice);

    CPPEmitHandler(const CLikeSourceEmitter::Desc& desc);

protected:
    void _emitVecMatMul(const UnownedStringSlice& funcName, const SpecializedOperation& specOp, CPPSourceEmitter* emitter);

    IRType* _getVecType(IRType* elementType, int elementCount);

    IRInst* _clone(IRInst* inst);
    IRType* _cloneType(IRType* type) { return (IRType*)_clone((IRInst*)type); }

    UnownedStringSlice _getFuncName(const SpecializedOperation& specOp);
    StringSlicePool::Handle _calcFuncName(const SpecializedOperation& specOp);

    UnownedStringSlice _getTypeName(IRType* type);
    StringSlicePool::Handle _calcTypeName(IRType* type);

    Dictionary<SpecializedOperation, StringSlicePool::Handle> m_specializeOperationNameMap;
    Dictionary<IRType*, StringSlicePool::Handle> m_typeNameMap;

    RefPtr<IRModule> m_uniqueModule;            ///< Store types/function sigs etc for output
    SharedIRBuilder m_sharedIRBuilder;
    IRBuilder m_irBuilder;

    Dictionary<IRInst*, IRInst*> m_cloneMap;

    Dictionary<IRType*, bool> m_typeEmittedMap;

    StringSlicePool m_slicePool;
};

class CPPSourceEmitter: public CLikeSourceEmitter
{
public:
    typedef CLikeSourceEmitter Super;

    SourceWriter* getSourceWriter() const { return m_writer; }

    CPPSourceEmitter(const Desc& desc, CPPEmitHandler* emitHandler);

protected:
    
    virtual void emitParameterGroupImpl(IRGlobalParam* varDecl, IRUniformParameterGroupType* type) SLANG_OVERRIDE;
    virtual void emitEntryPointAttributesImpl(IRFunc* irFunc, EntryPointLayout* entryPointLayout) SLANG_OVERRIDE;
    virtual void emitSimpleTypeImpl(IRType* type) SLANG_OVERRIDE;
    virtual void emitVectorTypeNameImpl(IRType* elementType, IRIntegerValue elementCount) SLANG_OVERRIDE;

    virtual bool tryEmitInstExprImpl(IRInst* inst, IREmitMode mode, const EmitOpInfo& inOuterPrec) SLANG_OVERRIDE;

    virtual void emitPreprocessorDirectivesImpl();

    void emitIntrinsicCallExpr(IRCall* inst, IRFunc* func, IREmitMode mode, EmitOpInfo const& inOuterPrec);

    RefPtr<CPPEmitHandler> m_emitHandler;
};

}
#endif
