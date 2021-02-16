// slang-hlsl-intrinsic-set.h
#pragma once

#include "slang-ir.h"
#include "slang-ir-insts.h"

#include "slang-ir-type-set.h"

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
#define SLANG_HLSL_INTRINSIC_OP(x) \
        x(Invalid, "", -1) \
        x(Init, "", -1) \
        \
        x(Mul, "*", 2) \
        x(Div, "/", 2) \
        x(Add, "+", 2) \
        x(Sub, "-", 2) \
        x(Lsh, "<<", 2) \
        x(Rsh, ">>", 2) \
        x(IRem, "%", 2) \
        x(FRem, "fmod", 2) \
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
        x(AsFloat, "asfloat", 1) \
        x(AsInt, "asint", -1) \
        x(AsUInt, "asuint", -1) \
        x(AsDouble, "asdouble", 2) \
        \
        x(ConstructConvert, "", 1) \
        x(ConstructFromScalar, "", 1) \
        \
        x(GetAt, "", 2) \
        /* end */

struct HLSLIntrinsic
{
    typedef HLSLIntrinsic ThisType;

    enum class Op : uint8_t
    {
#define SLANG_HLSL_INTRINSIC_OP_ENUM(name, hlslName, numOperands)  name,
        SLANG_HLSL_INTRINSIC_OP(SLANG_HLSL_INTRINSIC_OP_ENUM)
    };

    struct Info
    {
        UnownedStringSlice name;                ///< The enum name
        UnownedStringSlice funcName;            ///< The HLSL function name (if there is one)
        int8_t numOperands;                     ///< -1 if can't be handled automatically via amount of params
    };

    bool operator==(const ThisType& rhs) const { return op == rhs.op && returnType == rhs.returnType && signatureType == rhs.signatureType; }
    bool operator!=(const ThisType& rhs) const { return !(*this == rhs); }

    static bool isTypeScalar(IRType* type)
    {
        // Strip off ptr if it's an operand type
        if (type->getOp() == kIROp_PtrType)
        {
            type = as<IRType>(type->getOperand(0));
        }
        // If any are vec or matrix, then we
        return !(type->getOp() == kIROp_MatrixType || type->getOp() == kIROp_VectorType);
    }

    bool isScalar() const
    {
        Index paramCount = Index(signatureType->getParamCount());
        for (Index i = 0; i < paramCount; ++i)
        {
            if (!isTypeScalar(signatureType->getParamType(i)))
            {
                return false;
            }
        }
        return isTypeScalar(returnType);
    }

    HashCode getHashCode() const { return combineHash(int(op), combineHash(Slang::getHashCode(returnType), Slang::getHashCode(signatureType))); }

    static const Info& getInfo(Op op) { return s_operationInfos[Index(op)]; }
    static const Info s_operationInfos[];

    Op op;
    IRType* returnType;
    IRFuncType* signatureType;              // Same as funcType, but has return type of void
};

/* A helper type that allows comparing pointers to HLSLIntrinsic types as if they are the values */
struct HLSLIntrinsicRef
{
    typedef HLSLIntrinsicRef ThisType;

    HashCode getHashCode() const { return m_intrinsic->getHashCode(); }
    bool operator==(const ThisType& rhs) const { return m_intrinsic == rhs.m_intrinsic || (*m_intrinsic == *rhs.m_intrinsic); }
    bool operator!=(const ThisType& rhs) const { return !(*this == rhs); }

    HLSLIntrinsicRef():m_intrinsic(nullptr) {}
    HLSLIntrinsicRef(const ThisType& rhs):m_intrinsic(rhs.m_intrinsic) {}
    HLSLIntrinsicRef(const HLSLIntrinsic* intrinsic): m_intrinsic(intrinsic) {}
    void operator=(const ThisType& rhs) { m_intrinsic = rhs.m_intrinsic; }

    const HLSLIntrinsic* m_intrinsic;
};

class HLSLIntrinsicOpLookup : public RefObject
{
public:
    typedef HLSLIntrinsic::Op Op;

    Op getOpFromTargetDecoration(IRInst* inInst);
    Op getOpByName(const UnownedStringSlice& slice);

    Op getOpForIROp(IRInst* inst);

    HLSLIntrinsicOpLookup();

        /// Given an IROp returns the Op equivalent or Op::Invalid if not found
    static Op getOpForIROp(IROp op);

protected:
    
    StringSlicePool m_slicePool;
    List<Op> m_sliceToOpMap;
};


/* This is used so as to try and use slangs type system to uniquely identify types and specializations on intrinsic.
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
class HLSLIntrinsicSet
{
public:
    typedef HLSLIntrinsic::Op Op;

    /* Note that calculating an intrinsic, the types will be added to the type set. That might mean subsequent code will
    emit those types being required, which may not be the case */

    void calcIntrinsic(Op op, IRType* returnType, IRType*const* args, Index argsCount, HLSLIntrinsic& out);
    void calcIntrinsic(Op op, IRInst* inst, Index argsCount, HLSLIntrinsic& out);
    void calcIntrinsic(Op op, IRType* returnType, IRUse* args, Index argCount, HLSLIntrinsic& out);
    void  calcIntrinsic(Op op, IRInst* inst, HLSLIntrinsic& out) { calcIntrinsic(op, inst, Index(inst->getOperandCount()), out); }

    SlangResult makeIntrinsic(IRInst* inst, HLSLIntrinsic& out);
    
    HLSLIntrinsic* add(const HLSLIntrinsic& intrinsic);

        /// Returns the intrinsic constructed if there is one from the inst. If not possible to construct returns nullptr.
    HLSLIntrinsic* add(IRInst* inst);

    void getIntrinsics(List<const HLSLIntrinsic*>& out) const;

    HLSLIntrinsicSet(IRTypeSet* typeSet, HLSLIntrinsicOpLookup* lookup);
    
protected:
    // All calcs must go through this choke point for some special case handling.
    // NOTE that this function must only be called with unique types (ie from the m_typeSet)
    void _calcIntrinsic(HLSLIntrinsic::Op op, IRType* returnType, IRType*const* inArgs, Index argsCount, HLSLIntrinsic& out);
    
    List<HLSLIntrinsic*> m_intrinsicsList;
    Dictionary<HLSLIntrinsicRef, HLSLIntrinsic*> m_intrinsicsDict;

    FreeList m_intrinsicFreeList;           ///< the storage for the intrinsics when they are in the map

    HLSLIntrinsicOpLookup* m_opLookup;
    IRTypeSet* m_typeSet;
};

} // namespace Slang
