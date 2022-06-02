// slang-hlsl-intrinsic-set.cpp
#include "slang-hlsl-intrinsic-set.h"

#include "slang-ir.h"
#include "slang-ir-insts.h"

namespace Slang
{

/* static */const HLSLIntrinsic::Info HLSLIntrinsic::s_operationInfos[] =
{
#define SLANG_HLSL_INTRINSIC_OP_INFO(x, funcName, numOperands) { UnownedStringSlice::fromLiteral(#x), UnownedStringSlice::fromLiteral(funcName), int8_t(numOperands)  },
    SLANG_HLSL_INTRINSIC_OP(SLANG_HLSL_INTRINSIC_OP_INFO)
};

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!! HLSLIntrinsicSet !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

HLSLIntrinsicSet::HLSLIntrinsicSet(IRTypeSet* typeSet, HLSLIntrinsicOpLookup* lookup):
    m_intrinsicFreeList(sizeof(HLSLIntrinsic), SLANG_ALIGN_OF(HLSLIntrinsic), 1024),
    m_typeSet(typeSet),
    m_opLookup(lookup)
{
}

static IRBasicType* _getElementType(IRType* type)
{
    switch (type->getOp())
    {
        case kIROp_VectorType:      type = static_cast<IRVectorType*>(type)->getElementType(); break;
        case kIROp_MatrixType:      type = static_cast<IRMatrixType*>(type)->getElementType(); break;
        default:                    break;
    }
    return dynamicCast<IRBasicType>(type);
}

void HLSLIntrinsicSet::_calcIntrinsic(HLSLIntrinsic::Op op, IRType* returnType, IRType*const* inArgs, Index argsCount, HLSLIntrinsic& out)
{
    IRBuilder& builder = m_typeSet->getBuilder();

    // Check all types belong to the module

    IRModule* module = builder.getModule();

    SLANG_UNUSED(module);
    SLANG_ASSERT(returnType->getModule() == module);

    for (Index i = 0; i < argsCount; ++i)
    {
        SLANG_ASSERT(inArgs[i]->getModule() == module);
    }

    // Set up the out
    out.op = op;
    out.returnType = returnType;

    switch (op)
    {
        case Op::GetAt:
        {
            IRType* argTypes[3];

            SLANG_ASSERT(argsCount == 2 || argsCount == 3);
            // TODO(JS):
            // HACK! GetAt can be from getElementPtr or from getElement. Get element ptr means the return type will be
            // a pointer. We don't want to deal with that, so strip it
            if (returnType->getOp() == kIROp_PtrType)
            {
                returnType = as<IRType>(returnType->getOperand(0));
            }

            // TODO(JS): Similarly for the input parameters 
            for (Index i = 0; i < argsCount; ++i)
            {
                IRType* argType = inArgs[i];

                if (argType->getOp() == kIROp_PtrType)
                {
                    argType = as<IRType>(argType->getOperand(0));
                }
                argTypes[i] = argType;
            }

            out.returnType = returnType;
            out.signatureType = builder.getFuncType(argsCount, argTypes, builder.getVoidType());
            break;
        }
        case Op::ConstructFromScalar:
        {
            //SLANG_ASSERT(argsCount == 1);
            SLANG_ASSERT(argsCount == 1);
            IRType* srcType = _getElementType(returnType);
            IRType* argTypes[2] = { returnType, srcType };

            out.signatureType = builder.getFuncType(2, argTypes, builder.getVoidType());
            break;
        }
        case Op::ConstructConvert:
        {
            // Make the return type a parameter, to make the signature take into account 
            SLANG_ASSERT(argsCount == 1);
            IRType* argTypes[2] = { returnType, inArgs[0] };

            out.signatureType = builder.getFuncType(2, argTypes, builder.getVoidType());
            break;
        }
        default:
        {
            out.signatureType = builder.getFuncType(argsCount, inArgs, builder.getVoidType());
            break;
        }
    }
}

void HLSLIntrinsicSet::calcIntrinsic(HLSLIntrinsic::Op op, IRType* returnType, IRType*const* inArgTypes, Index argCount, HLSLIntrinsic& out)
{
    returnType = m_typeSet->getType(returnType);

    if (argCount <= 8)
    {
        IRType* args[8];
        for (Index i = 0; i < argCount; ++i)
        {
            args[i] = m_typeSet->getType(inArgTypes[i]);
        }
        _calcIntrinsic(op, returnType, args, argCount, out);
    }
    else
    {
        List<IRType*> args;
        args.setCount(argCount);

        for (Index i = 0; i < argCount; ++i)
        {
            args[i] = m_typeSet->getType(inArgTypes[i]);
        }
        _calcIntrinsic(op, returnType, args.getBuffer(), argCount, out);
    }
}

void HLSLIntrinsicSet::calcIntrinsic(HLSLIntrinsic::Op op, IRInst* inst, Index operandCount, HLSLIntrinsic& out)
{
    IRType* returnType = m_typeSet->getType(inst->getDataType());
    if (operandCount <= 8)
    {
        IRType* argTypes[8];
        for (Index i = 0; i < operandCount; ++i)
        {
            auto operand = inst->getOperand(i);
            argTypes[i] = m_typeSet->getType(operand->getDataType());
        }
        _calcIntrinsic(op, returnType, argTypes, operandCount, out);
    }
    else
    {
        List<IRType*> argTypes;
        argTypes.setCount(operandCount);

        for (Index i = 0; i < operandCount; ++i)
        {
            auto operand = inst->getOperand(i);
            argTypes[i] = m_typeSet->getType(operand->getDataType());
        }
        _calcIntrinsic(op, returnType, argTypes.getBuffer(), operandCount, out);
    }
}

void HLSLIntrinsicSet::calcIntrinsic(HLSLIntrinsic::Op op, IRType* returnType, IRUse* inArgs, Index argCount, HLSLIntrinsic& out)
{
    returnType = m_typeSet->getType(returnType);

    if (argCount <= 8)
    {
        IRType* argTypes[8];

        for (Index i = 0; i < argCount; ++i)
        {
            auto operand = inArgs[i].get();
            argTypes[i] = m_typeSet->getType(operand->getDataType());
        }
        _calcIntrinsic(op, returnType, argTypes, argCount, out);
    }
    else
    {
        List<IRType*> argTypes;
        argTypes.setCount(argCount);

        for (Index i = 0; i < argCount; ++i)
        {
            auto operand = inArgs[i].get();
            argTypes[i] = m_typeSet->getType(operand->getDataType());
        }
        _calcIntrinsic(op, returnType, argTypes.getBuffer(), argCount, out);
    }
}

HLSLIntrinsic* HLSLIntrinsicSet::add(IRInst* inst)
{
    HLSLIntrinsic intrinsic;
    if (SLANG_SUCCEEDED(makeIntrinsic(inst, intrinsic)))
    {
        return add(intrinsic);
    }
    return nullptr;
}

SlangResult HLSLIntrinsicSet::makeIntrinsic(IRInst* inst, HLSLIntrinsic& out)
{
    // Mark as invalid... 
    out.op = Op::Invalid;

    {
        // See if we can just directly convert
        Op op = HLSLIntrinsicOpLookup::getOpForIROp(inst->getOp());


        // HACK: some cases we want to stop handling via the synthesis
        // path, but only for vector and matrix types (not scalars).
        //
        switch( op )
        {
        default: break;

        case Op::AsFloat:
        case Op::AsInt:
        case Op::AsUInt:
            // Note: the `any()`/`all()` case can't be handled via a stdlib definition
            // right now because `bool` vectors map to `int` vectors on the CUDA
            // path, so that the generated `geAt` operation is incorrect.
            //
//        case Op::Any:
//        case Op::All:
            {
                IRType* srcType = inst->getOperand(0)->getDataType();
                switch( srcType->getOp() )
                {
                default:
                    break;

                case kIROp_VectorType:
                case kIROp_MatrixType:
                    return SLANG_FAIL;
                }
            }
            break;
        }


        if (op != Op::Invalid)
        {
            calcIntrinsic(op, inst, inst->getOperandCount(), out);
            return SLANG_OK;
        }
    }

    // All the special cases
    switch (inst->getOp())
    {
        case kIROp_constructVectorFromScalar:
        {
            SLANG_ASSERT(inst->getOperandCount() == 1);
            calcIntrinsic(Op::ConstructFromScalar, inst, 1, out);
            return SLANG_OK;
        }
        case kIROp_Construct:
        {
            IRType* dstType = inst->getDataType();
            IRType* srcType = inst->getOperand(0)->getDataType();

            if ((dstType->getOp() == kIROp_VectorType || dstType->getOp() == kIROp_MatrixType) &&
                inst->getOperandCount() == 1)
            {
                if (as<IRBasicType>(srcType))
                {
                    calcIntrinsic(Op::ConstructFromScalar, inst, out);
                }
                else
                {
                    SLANG_ASSERT(m_typeSet->getType(dstType) != m_typeSet->getType(srcType));
                    // If it's constructed from a type conversion
                    calcIntrinsic(Op::ConstructConvert, inst, out);
                }
                return SLANG_OK;
            }
            else
            {
                // If we are constructing a basic type, we don't need an Op::Init
                if (!IRBasicType::isaImpl(dstType->getOp()))
                {
                    // Emit the 'init' intrinsic
                    calcIntrinsic(Op::Init, inst, inst->getOperandCount(), out);
                    return SLANG_OK;
                }
            }
            return SLANG_FAIL;
        }
        case kIROp_makeVector:
        {
            if (inst->getOperandCount() == 1 && as<IRBasicType>(inst->getOperand(0)->getDataType()))
            {
                // This is make from scalar
                calcIntrinsic(Op::ConstructFromScalar, inst, out);
            }
            else
            {
                calcIntrinsic(Op::Init, inst, inst->getOperandCount(), out);
            }
            return SLANG_OK;
        }
        case kIROp_MakeMatrix:
        {
            // We only emit as if it has one operand, but we can tell how many it actually has from the return type
            calcIntrinsic(Op::Init, inst, inst->getOperandCount(), out);
            return SLANG_OK;
        }
        case kIROp_swizzle:
        {
            // We don't need to add swizzle function, but we do output the need for some other functions 

            // For C++ we don't need to emit a swizzle function
            // For C we need a construction function
            auto swizzleInst = static_cast<IRSwizzle*>(inst);

            IRInst* baseInst = swizzleInst->getBase();
            IRType* baseType = baseInst->getDataType();

            // If we are swizzling from a built in type, 
            if (as<IRBasicType>(baseType))
            {
                // We can swizzle a scalar type to be a vector, or just a scalar
                IRType* dstType = swizzleInst->getDataType();
                if (!as<IRBasicType>(dstType))
                {
                    // If it's a scalar make sure we have construct from scalar, because we will want to use that
                    SLANG_ASSERT(dstType->getOp() == kIROp_VectorType);
                    IRType* argTypes[] = { baseType };
                    calcIntrinsic(Op::ConstructFromScalar, inst->getDataType(), argTypes, 1,  out);
                    return SLANG_OK;
                }
            }
            else
            {
                const Index elementCount = Index(swizzleInst->getElementCount());
                if (elementCount >= 1)
                {
                    // Will need to generate a swizzle method
                    calcIntrinsic(Op::Swizzle, inst, out);
                    return SLANG_OK;
                }
            }
            break;
        }
        case kIROp_getElement:
        {
            IRInst* target = inst->getOperand(0);
            IRType* targetType = target->getDataType();
            if (targetType->getOp() == kIROp_VectorType || targetType->getOp() == kIROp_MatrixType)
            {
                // Specially handle this
                calcIntrinsic(Op::GetAt, inst, out);
                return SLANG_OK;
            }
            break;
        }
        case kIROp_getElementPtr:
        {
            IRInst* target = inst->getOperand(0);
            IRType* targetType = target->getDataType();

            if (auto ptrType = as<IRPtrType>(targetType))
            {
                targetType = as<IRType>(ptrType->getOperand(0));
                if (targetType->getOp() == kIROp_VectorType || targetType->getOp() == kIROp_MatrixType)
                {
                    // Specially handle this
                    calcIntrinsic(Op::GetAt, inst, out);
                    return SLANG_OK;
                }
            }
            break;
        }
        case kIROp_Call:
        {
            IRCall* callInst = (IRCall*)inst;
            auto funcValue = callInst->getCallee();

            const Op op = m_opLookup->getOpFromTargetDecoration(funcValue);
            if (op != Op::Invalid)
            {
                calcIntrinsic(op, inst->getDataType(), callInst->getArgs(), callInst->getArgCount(), out);
                return SLANG_OK;
            }
            break;
        }

        default: break;
    }

    return SLANG_FAIL;
}

void HLSLIntrinsicSet::getIntrinsics(List<const HLSLIntrinsic*>& out) const
{
    for (auto& intrinsic : m_intrinsicsList)
    {
        out.add(intrinsic);
    }
}

HLSLIntrinsic* HLSLIntrinsicSet::add(const HLSLIntrinsic& intrinsic)
{
    // Make sure it's valid(!)
    SLANG_ASSERT(intrinsic.op != Op::Invalid);

    HLSLIntrinsic* copy = (HLSLIntrinsic*)m_intrinsicFreeList.allocate();
    *copy = intrinsic;
    HLSLIntrinsicRef ref(copy);
    HLSLIntrinsic** found =  m_intrinsicsDict.TryGetValueOrAdd(ref, copy);
    if (found)
    {
        // If we have found an intrinsic, we can free the copy
        m_intrinsicFreeList.deallocate(copy);
        return *found;
    }

    // If we are adding an intrinsic for the first time,
    // it should be added to the deduplicated list
    m_intrinsicsList.add(copy);

    return copy;
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!! HLSLIntrinsicOpLookup !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

HLSLIntrinsicOpLookup::HLSLIntrinsicOpLookup():
    m_slicePool(StringSlicePool::Style::Default)
{
    // Add all the operations with names (not ops like -, / etc) to the lookup map
    for (int i = 0; i < SLANG_COUNT_OF(HLSLIntrinsic::s_operationInfos); ++i)
    {
        const auto& info = HLSLIntrinsic::getInfo(Op(i));
        UnownedStringSlice slice = info.funcName;

        if (slice.getLength() > 0 && slice[0] >= 'a' && slice[0] <= 'z')
        {
            auto handle = m_slicePool.add(slice);
            Index index = Index(handle);
            // Make sure there is space
            if (index >= m_sliceToOpMap.getCount())
            {
                Index oldSize = m_sliceToOpMap.getCount();
                m_sliceToOpMap.setCount(index + 1);
                for (Index j = oldSize; j < index; j++)
                {
                    m_sliceToOpMap[j] = Op::Invalid;
                }
            }
            m_sliceToOpMap[index] = Op(i);
        }
    }
}

HLSLIntrinsic::Op HLSLIntrinsicOpLookup::getOpByName(const UnownedStringSlice& slice)
{
    const Index index = m_slicePool.findIndex(slice);
    return (index >= 0 && index < m_sliceToOpMap.getCount()) ? m_sliceToOpMap[index] : Op::Invalid;
}

static IRInst* _getSpecializedValue(IRSpecialize* specInst)
{
    auto base = specInst->getBase();
    auto baseGeneric = as<IRGeneric>(base);
    if (!baseGeneric)
        return base;

    auto lastBlock = baseGeneric->getLastBlock();
    if (!lastBlock)
        return base;

    auto returnInst = as<IRReturn>(lastBlock->getTerminator());
    if (!returnInst)
        return base;

    return returnInst->getVal();
}

HLSLIntrinsic::Op HLSLIntrinsicOpLookup::getOpFromTargetDecoration(IRInst* inInst)
{
    // An intrinsic generic function will be invoked through a `specialize` instruction,
    // so the callee won't directly be the thing that is decorated. We will look up
    // through specializations until we can see the actual thing being called.
    //
    IRInst* inst = inInst;
    while (auto specInst = as<IRSpecialize>(inst))
    {
        inst = _getSpecializedValue(specInst);

        // If `getSpecializedValue` can't find the result value
        // of the generic being specialized, then it returns
        // the original instruction. This would be a disaster
        // for use because this loop would go on forever.
        //
        // This case should never happen if the stdlib is well-formed
        // and the compiler is doing its job right.
        //
        SLANG_ASSERT(inst != specInst);
    }

    // We are just looking for the original name so we can match against it
    for (auto dd : inst->getDecorations())
    {
        if (auto decor = as<IRTargetIntrinsicDecoration>(dd))
        {
            // TODO(JS): Should confirm that we'll always have this entry - which we need for lookups to work (we need the name
            // not a targets transformation)
            // 
            // It turns out that addCatchAllIntrinsicDecorationIfNeeded will add a target intrinsic with the
            // original HLSL name, which has an empty `CapabilitySet`.
            // 
            // It's not 100% clear this covers all the cases, but for now lets go with that
            if (decor->getTargetCaps().isEmpty())
            {
                Op op = getOpByName(decor->getDefinition());
                if (op != Op::Invalid)
                {
                    return op;
                }
            }
        }
    }

    return Op::Invalid;
}

HLSLIntrinsic::Op HLSLIntrinsicOpLookup::getOpForIROp(IRInst* inst)
{
    switch (inst->getOp())
    {
        case kIROp_Call:
        {
            return getOpFromTargetDecoration(inst);
        }
        default: break;
    }
    return getOpForIROp(inst->getOp());
}

/* static */HLSLIntrinsic::Op HLSLIntrinsicOpLookup::getOpForIROp(IROp op)
{
    switch (op)
    {
        case kIROp_Add:     return Op::Add;
        case kIROp_Mul:     return Op::Mul;
        case kIROp_Sub:     return Op::Sub;
        case kIROp_Div:     return Op::Div;
        case kIROp_Lsh:     return Op::Lsh;
        case kIROp_Rsh:     return Op::Rsh;
        case kIROp_IRem:    return Op::IRem;
        case kIROp_FRem:    return Op::FRem;

        case kIROp_Eql:     return Op::Eql;
        case kIROp_Neq:     return Op::Neq;
        case kIROp_Greater: return Op::Greater;
        case kIROp_Less:    return Op::Less;
        case kIROp_Geq:     return Op::Geq;
        case kIROp_Leq:     return Op::Leq;

        case kIROp_BitAnd:  return Op::BitAnd;
        case kIROp_BitXor:  return Op::BitXor;
        case kIROp_BitOr:   return Op::BitOr;

        case kIROp_And:     return Op::And;
        case kIROp_Or:      return Op::Or;

        case kIROp_Neg:     return Op::Neg;
        case kIROp_Not:     return Op::Not;
        case kIROp_BitNot:  return Op::BitNot;

        case kIROp_constructVectorFromScalar: return Op::ConstructFromScalar;

        default:            return Op::Invalid;
    }
}

}
