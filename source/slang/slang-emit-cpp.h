// slang-emit-cpp.h
#ifndef SLANG_EMIT_CPP_H
#define SLANG_EMIT_CPP_H

#include "slang-emit-c-like.h"
#include "slang-ir-clone.h"

#include "../core/slang-string-slice-pool.h"

namespace Slang
{

/* TODO(JS): Note that there are multiple methods to handle 'construction' operations. That is because 'construct' is used as a kind of
generic 'construction' for built in types including vectors and matrices.

For the moment the cpp emit code, determines what kind of construct is needed, and has special handling for ConstructConvert and
ConstructFromScalar. 

That currently we do not see constructVectorFromScalar - for example when we do...

int2 fromScalar = 1;

This appears as a construction from an int.

That the better thing to do would be that there were IR instructions for the specific types of construction. I suppose there is a question
about whether there should be separate instructions for vector/matrix, or emit code should just use the destination type. In practice I think
it's fine that there isn't an instruction separating vector/matrix. That being the case I guess we arguably don't need constructVectorFromScalar,
just constructXXXFromScalar. Would be good if there was a suitable name to encompass vector/matrix. 
*/

#define SLANG_CPP_INTRINSIC_OP(x) \
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
        x(AsUInt, "asuint", 1) \
        \
        x(ConstructConvert, "", 1) \
        x(ConstructFromScalar, "", 1) 
  

class CPPSourceEmitter: public CLikeSourceEmitter
{
public:
    typedef CLikeSourceEmitter Super;

#define SLANG_CPP_INTRINSIC_OP_ENUM(x, op, numOperands) x,
    enum class IntrinsicOp
    {
        SLANG_CPP_INTRINSIC_OP(SLANG_CPP_INTRINSIC_OP_ENUM)
    };

    struct OperationInfo
    {
        UnownedStringSlice name;
        UnownedStringSlice funcName;
        int8_t numOperands;                     ///< -1 if can't be handled automatically via amount of params
    };

    struct SpecializedIntrinsic
    {
        typedef SpecializedIntrinsic ThisType;

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

        IntrinsicOp op;
        IRType* returnType;
        IRFuncType* signatureType;              // Same as funcType, but has return type of void
    };

    struct TypeDimension
    {
        bool isScalar() const { return rowCount <= 1 && colCount <= 1; }

        int rowCount;
        int colCount;
    };

    virtual SpecializedIntrinsic getSpecializedOperation(IntrinsicOp op, IRType*const* argTypes, int argTypesCount, IRType* retType);
    virtual void useType(IRType* type);
    virtual void emitCall(const SpecializedIntrinsic& specOp, IRInst* inst, const IRUse* operands, int numOperands, const EmitOpInfo& inOuterPrec);
    virtual void emitTypeDefinition(IRType* type);
    virtual void emitSpecializedOperationDefinition(const SpecializedIntrinsic& specOp);
    
    void emitOperationCall(IntrinsicOp op, IRInst* inst, IRUse* operands, int operandCount, IRType* retType, const EmitOpInfo& inOuterPrec);

    static UnownedStringSlice getBuiltinTypeName(IROp op);

    static const OperationInfo& getOperationInfo(IntrinsicOp op);

    static IntrinsicOp getOperation(IROp op);

    IntrinsicOp getOperationByName(const UnownedStringSlice& slice);

    SourceWriter* getSourceWriter() const { return m_writer; }

    CPPSourceEmitter(const Desc& desc);

protected:

    // Implement CLikeSourceEmitter interface
    virtual void emitParameterGroupImpl(IRGlobalParam* varDecl, IRUniformParameterGroupType* type) SLANG_OVERRIDE;
    virtual void emitEntryPointAttributesImpl(IRFunc* irFunc, EntryPointLayout* entryPointLayout) SLANG_OVERRIDE;
    virtual void emitSimpleTypeImpl(IRType* type) SLANG_OVERRIDE;
    virtual void emitTypeImpl(IRType* type, const StringSliceLoc* nameLoc) SLANG_OVERRIDE;
    virtual void emitVectorTypeNameImpl(IRType* elementType, IRIntegerValue elementCount) SLANG_OVERRIDE;
    virtual bool tryEmitInstExprImpl(IRInst* inst, const EmitOpInfo& inOuterPrec) SLANG_OVERRIDE;
    virtual void emitPreprocessorDirectivesImpl() SLANG_OVERRIDE;
    virtual void emitSimpleValueImpl(IRInst* value) SLANG_OVERRIDE;
    virtual void emitModuleImpl(IRModule* module) SLANG_OVERRIDE;
    virtual void emitSimpleFuncImpl(IRFunc* func) SLANG_OVERRIDE;
    virtual void emitOperandImpl(IRInst* inst, EmitOpInfo const&  outerPrec) SLANG_OVERRIDE;
    virtual void emitParamTypeImpl(IRType* type, String const& name) SLANG_OVERRIDE;

    virtual bool tryEmitGlobalParamImpl(IRGlobalParam* varDecl, IRType* varType) SLANG_OVERRIDE;

    void emitIntrinsicCallExpr(IRCall* inst, IRFunc* func, EmitOpInfo const& inOuterPrec);

    void _emitVecMatMulDefinition(const UnownedStringSlice& funcName, const SpecializedIntrinsic& specOp);

    void _emitAryDefinition(const SpecializedIntrinsic& specOp);

    // Really we don't want any of these defined like they are here, they should be defined in slang stdlib 
    void _emitAnyAllDefinition(const UnownedStringSlice& funcName, const SpecializedIntrinsic& specOp);
    void _emitCrossDefinition(const UnownedStringSlice& funcName, const SpecializedIntrinsic& specOp);
    void _emitLengthDefinition(const UnownedStringSlice& funcName, const SpecializedIntrinsic& specOp);
    void _emitNormalizeDefinition(const UnownedStringSlice& funcName, const SpecializedIntrinsic& specOp);
    void _emitReflectDefinition(const UnownedStringSlice& funcName, const SpecializedIntrinsic& specOp);
    void _emitConstructConvertDefinition(const UnownedStringSlice& funcName, const SpecializedIntrinsic& specOp);
    void _emitConstructFromScalarDefinition(const UnownedStringSlice& funcName, const SpecializedIntrinsic& specOp);

    void _emitSignature(const UnownedStringSlice& funcName, const SpecializedIntrinsic& specOp);

    void _emitInOutParamType(IRType* type, String const& name, IRType* valueType);

    UnownedStringSlice _getAndEmitSpecializedOperationDefinition(IntrinsicOp op, IRType*const* argTypes, Int argCount, IRType* retType);

    static TypeDimension _getTypeDimension(IRType* type, bool vecSwap);
    static void _emitAccess(const UnownedStringSlice& name, const TypeDimension& dimension, int row, int col, SourceWriter* writer);

    IRType* _getVecType(IRType* elementType, int elementCount);

    IRInst* _clone(IRInst* inst);
    IRType* _cloneType(IRType* type) { return (IRType*)_clone((IRInst*)type); }

    StringSlicePool::Handle _calcScalarFuncName(IntrinsicOp op, IRBasicType* type);
    UnownedStringSlice _getScalarFuncName(IntrinsicOp operation, IRBasicType* scalarType);

    UnownedStringSlice _getFuncName(const SpecializedIntrinsic& specOp);
    StringSlicePool::Handle _calcFuncName(const SpecializedIntrinsic& specOp);

    UnownedStringSlice _getTypeName(IRType* type);
    StringSlicePool::Handle _calcTypeName(IRType* type);

    SlangResult _calcTypeName(IRType* type, CodeGenTarget target, StringBuilder& out);

    SlangResult _calcTextureTypeName(IRTextureTypeBase* texType, StringBuilder& outName);

    Dictionary<SpecializedIntrinsic, StringSlicePool::Handle> m_intrinsicNameMap;
    Dictionary<IRType*, StringSlicePool::Handle> m_typeNameMap;

    /* This is used so as to try and use slangs type system to uniquely identify types and specializations on intrinsice.
    That we want to have a pointer to a type be unique, and slang supports this through the m_sharedIRBuilder. BUT for this to
    work all work on the module must use the same sharedIRBuilder, and that appears to not be the case in terms
    of other passes.
    Even if it was the case when we may want to add types as part of emitting, we can't use the previously used
    shared builder, so again we end up with pointers to the same things not being the same thing.

    To work around this we clone types we want to use as keys into the 'unique module'.
    This is not necessary for all types though - as we assume nominal types *must* have unique pointers (that is the
    definition of nominal).

    This could be handled in other ways (for example not testing equality on pointer equality). Anyway for now this
    works, but probably needs to be handled in a better way. The better way may involve having guarantees about equality
    enabled in other code generation and making de-duping possible in emit code.

    Note that one pro for this approach is that it does not alter the source module. That as it stands it's not necessary
    for the source module to be immutable, because it is created for emitting and then discarded. 
     */
    RefPtr<IRModule> m_uniqueModule;            
    SharedIRBuilder m_sharedIRBuilder;
    IRBuilder m_irBuilder;

    Dictionary<IRInst*, IRInst*> m_cloneMap;

    Dictionary<IRType*, bool> m_typeEmittedMap;
    Dictionary<SpecializedIntrinsic, bool> m_intrinsicEmittedMap;

    // Maps from a name (in the form of a handle/index from m_slicePool) to an operation
    List<IntrinsicOp> m_intrinsicOpMap;

    StringSlicePool m_slicePool;
};

}
#endif
