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
        x(Invalid, "", -1) \
        x(Init, "", -1) \
        \
        x(Mul, "*", 2) \
        x(Div, "/", 2) \
        x(Add, "+", 2) \
        x(Sub, "-", 2) \
        x(Lsh, "<<", 2) \
        x(Rsh, ">>", 2) \
        x(Mod, "%", 2) \
        \
        x(Eql, "==", 2) \
        x(Neq, "!=", 2) \
        x(Greater, ">", 2) \
        x(Less, "<", 2) \
        x(Geq, ">=", 2) \
        x(Leq, "<=", 2) \
        \
        x(BitAnd, "&", 2) \
        x(BitXor, "^", 2) \
        x(BitOr, "|" , 2) \
        \
        x(And, "&&", 2) \
        x(Or, "||", 2) \
        \
        x(Neg, "-", 1) \
        x(Not, "!", 1) \
        x(BitNot, "~", 1) \
        \
        x(Any, "any", 1) \
        x(All, "all", 1) \
        \
        x(Swizzle, "", -1) \
        \
        x(Dot, "dot", 2) \
        x(VecMatMul, "mul", 2) \
        \
        x(Normalize, "normalize", 1) \
        x(Length, "length", 1) \
        \
        x(Sin, "sin", 1) \
        x(Cos, "cos", 1) \
        x(Tan, "tan", 1) \
        \
        x(ArcSin, "asin", 1) \
        x(ArcCos, "acos", 1) \
        x(ArcTan, "atan", 1) \
        \
        x(ArcTan2, "atan2", 2) \
        x(SinCos, "sincos", 3) \
        \
        x(Rcp, "rcp", 1) \
        x(Sign, "sign", 1) \
        x(Saturate, "saturate", 1) \
        x(Frac, "frac", 1) \
        \
        x(Ceil, "ceil", 1) \
        x(Floor, "floor", 1) \
        x(Trunc, "trunc", 1) \
        \
        x(Sqrt, "sqrt", 1) \
        x(RecipSqrt, "rsqrt", 1) \
        \
        x(Exp2, "exp2", 1) \
        x(Exp, "exp", 1) \
        \
        x(Abs, "abs", 1) \
        \
        x(Min, "min", 2) \
        x(Max, "max", 2) \
        x(Pow, "pow", 2) \
        x(FMod, "fmod", 2) \
        x(Cross, "cross", 2) \
        x(Reflect, "reflect", 2) \
        \
        x(SmoothStep, "smoothstep", 3) \
        x(Lerp, "lerp", 3) \
        x(Clamp, "clamp", 3) \
        x(Step, "step", 2) \
        \
        x(AsFloat, "asfloat", 1) \
        x(AsInt, "asint", 1) \
        x(AsUInt, "asuint", 1)
        
class CPPEmitHandler: public RefObject
{
public:
#define SLANG_CPP_OPERATION_ENUM(x, op, numOperands) x,

    enum class Operation
    {
        SLANG_CPP_OPERATION(SLANG_CPP_OPERATION_ENUM)
    };

    struct OperationInfo
    {
        UnownedStringSlice name;
        UnownedStringSlice funcName;
        int8_t numOperands;                     ///< -1 if can't be handled automatically via amount of params
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

    struct Dimension
    {
        bool isScalar() const { return rowCount <= 1 && colCount <= 1; }

        int rowCount;
        int colCount;
    };

    virtual SpecializedOperation getSpecializedOperation(Operation op, IRType*const* argTypes, int argTypesCount, IRType* retType);
    virtual void useType(IRType* type);
    virtual void emitCall(const SpecializedOperation& specOp, IRInst* inst, const IRUse* operands, int numOperands, CLikeSourceEmitter::IREmitMode mode, const EmitOpInfo& inOuterPrec, CPPSourceEmitter* emitter);
    virtual void emitType(IRType* type, CPPSourceEmitter* emitter);
    virtual void emitTypeDefinition(IRType* type, CPPSourceEmitter* emitter);
    virtual void emitSpecializedOperationDefinition(const SpecializedOperation& specOp, CPPSourceEmitter* emitter);
    virtual void emitVectorTypeName(IRType* elementType, int elementCount, CPPSourceEmitter* emitter);

    virtual void emitPreamble(CPPSourceEmitter* emitter);

    void emitOperationCall(Operation op, IRInst* inst, IRUse* operands, int operandCount, IRType* retType, CLikeSourceEmitter::IREmitMode mode, const EmitOpInfo& inOuterPrec, CPPSourceEmitter* emitter);

    static UnownedStringSlice getBuiltinTypeName(IROp op);

    static const OperationInfo& getOperationInfo(Operation op);
    
    static Operation getOperation(IROp op);

    Operation getOperationByName(const UnownedStringSlice& slice);

    CPPEmitHandler(const CLikeSourceEmitter::Desc& desc);

protected:
    void _emitVecMatMulDefinition(const UnownedStringSlice& funcName, const SpecializedOperation& specOp, CPPSourceEmitter* emitter);

    void _emitAryDefinition(const SpecializedOperation& specOp, CPPSourceEmitter* emitter);

    // Really we don't want any of these defined like they are here, they should be defined in slang stdlib 
    void _emitAnyAllDefinition(const UnownedStringSlice& funcName, const SpecializedOperation& specOp, CPPSourceEmitter* emitter);
    void _emitCrossDefinition(const UnownedStringSlice& funcName, const SpecializedOperation& specOp, CPPSourceEmitter* emitter);
    void _emitLengthDefinition(const UnownedStringSlice& funcName, const SpecializedOperation& specOp, CPPSourceEmitter* emitter);
    void _emitNormalizeDefinition(const UnownedStringSlice& funcName, const SpecializedOperation& specOp, CPPSourceEmitter* emitter);
    void _emitReflectDefinition(const UnownedStringSlice& funcName, const SpecializedOperation& specOp, CPPSourceEmitter* emitter);

    void _emitSignature(const UnownedStringSlice& funcName, const SpecializedOperation& specOp, CPPSourceEmitter* emitter);
    
    UnownedStringSlice _getAndEmitSpecializedOperationDefinition(Operation op, IRType*const* argTypes, Int argCount, IRType* retType, CPPSourceEmitter* emitter);

    static Dimension _getDimension(IRType* type, bool vecSwap);
    static void _emitAccess(const UnownedStringSlice& name, const Dimension& dimension, int row, int col, SourceWriter* writer);
    
    IRType* _getVecType(IRType* elementType, int elementCount);

    IRInst* _clone(IRInst* inst);
    IRType* _cloneType(IRType* type) { return (IRType*)_clone((IRInst*)type); }

    StringSlicePool::Handle _calcScalarFuncName(Operation op, IRBasicType* type);
    UnownedStringSlice _getScalarFuncName(Operation operation, IRBasicType* scalarType);
    
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
    Dictionary<SpecializedOperation, bool> m_operationEmittedMap;

    // Maps from a name to an operation
    List<Operation> m_operationMap;  

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
