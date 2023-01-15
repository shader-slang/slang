#include "slang-ir-util.h"
#include "slang-ir-insts.h"
#include "slang-ir-clone.h"
#include "slang-ir-dce.h"

namespace Slang
{

bool isPointerOfType(IRInst* type, IROp opCode)
{
    if (auto ptrType = as<IRPtrTypeBase>(type))
    {
        return ptrType->getValueType() && ptrType->getValueType()->getOp() == opCode;
    }
    return false;
}

Dictionary<IRInst*, IRInst*> buildInterfaceRequirementDict(IRInterfaceType* interfaceType)
{
    Dictionary<IRInst*, IRInst*> result;
    for (UInt i = 0; i < interfaceType->getOperandCount(); i++)
    {
        auto entry = as<IRInterfaceRequirementEntry>(interfaceType->getOperand(i));
        if (!entry) continue;
        result[entry->getRequirementKey()] = entry->getRequirementVal();
    }
    return result;
}

bool isPointerOfType(IRInst* type, IRInst* elementType)
{
    if (auto ptrType = as<IRPtrTypeBase>(type))
    {
        return ptrType->getValueType() && isTypeEqual(ptrType->getValueType(), (IRType*)elementType);
    }
    return false;
}

bool isPtrToClassType(IRInst* type)
{
    return isPointerOfType(type, kIROp_ClassType);
}

bool isPtrToArrayType(IRInst* type)
{
    return isPointerOfType(type, kIROp_ArrayType) || isPointerOfType(type, kIROp_UnsizedArrayType);
}


bool isComInterfaceType(IRType* type)
{
    if (!type) return false;
    if (type->findDecoration<IRComInterfaceDecoration>() ||
        type->getOp() == kIROp_ComPtrType)
    {
        return true;
    }
    if (auto witnessTableType = as<IRWitnessTableTypeBase>(type))
    {
        return isComInterfaceType((IRType*)witnessTableType->getConformanceType());
    }
    if (auto ptrType = as<IRNativePtrType>(type))
    {
        auto valueType = ptrType->getValueType();
        return valueType->findDecoration<IRComInterfaceDecoration>() != nullptr;
    }

    return false;
}

IROp getTypeStyle(IROp op)
{
    switch (op)
    {
    case kIROp_VoidType:
    case kIROp_BoolType:
    {
        return op;
    }
    case kIROp_Int8Type:
    case kIROp_Int16Type:
    case kIROp_IntType:
    case kIROp_UInt8Type:
    case kIROp_UInt16Type:
    case kIROp_UIntType:
    case kIROp_Int64Type:
    case kIROp_UInt64Type:
    case kIROp_IntPtrType:
    case kIROp_UIntPtrType:
    {
        // All int like 
        return kIROp_IntType;
    }
    case kIROp_HalfType:
    case kIROp_FloatType:
    case kIROp_DoubleType:
    {
        // All float like
        return kIROp_FloatType;
    }
    default: return kIROp_Invalid;
    }
}

IROp getTypeStyle(BaseType op)
{
    switch (op)
    {
    case BaseType::Void:
        return kIROp_VoidType;
    case BaseType::Bool:
        return kIROp_BoolType;
    case BaseType::Char:
    case BaseType::Int8:
    case BaseType::Int16:
    case BaseType::Int:
    case BaseType::Int64:
    case BaseType::IntPtr:
    case BaseType::UInt8:
    case BaseType::UInt16:
    case BaseType::UInt:
    case BaseType::UInt64:
    case BaseType::UIntPtr:
        return kIROp_IntType;
    case BaseType::Half:
    case BaseType::Float:
    case BaseType::Double:
        return kIROp_FloatType;
    default:
        return kIROp_Invalid;
    }
}

IRInst* specializeWithGeneric(IRBuilder& builder, IRInst* genericToSpecialize, IRGeneric* userGeneric)
{
    List<IRInst*> genArgs;
    for (auto param : userGeneric->getFirstBlock()->getParams())
    {
        genArgs.add(param);
    }
    return builder.emitSpecializeInst(
        builder.getTypeKind(),
        genericToSpecialize,
        (UInt)genArgs.getCount(),
        genArgs.getBuffer());
}

IRInst* maybeSpecializeWithGeneric(IRBuilder& builder, IRInst* genericToSpecailize, IRInst* userGeneric)
{
    if (auto gen = as<IRGeneric>(userGeneric))
    {
        if (auto toSpecialize = as<IRGeneric>(genericToSpecailize))
        {
            return specializeWithGeneric(builder, toSpecialize, gen);
        }
    }
    return genericToSpecailize;
}

IRInst* hoistValueFromGeneric(IRBuilder& inBuilder, IRInst* value, IRInst*& outSpecializedVal, bool replaceExistingValue)
{
    auto outerGeneric = as<IRGeneric>(findOuterGeneric(value));
    if (!outerGeneric) return value;
    IRBuilder builder = inBuilder;
    builder.setInsertBefore(outerGeneric);
    auto newGeneric = builder.emitGeneric();
    builder.setInsertInto(newGeneric);
    builder.emitBlock();
    IRInst* newResultVal = nullptr;

    // Clone insts in outerGeneric up until `value`.
    IRCloneEnv cloneEnv;
    for (auto inst : outerGeneric->getFirstBlock()->getChildren())
    {
        auto newInst = cloneInst(&cloneEnv, &builder, inst);
        if (inst == value)
        {
            builder.emitReturn(newInst);
            newResultVal = newInst;
            break;
        }
    }
    SLANG_RELEASE_ASSERT(newResultVal);
    if (newResultVal->getOp() == kIROp_Func)
    {
        IRBuilder subBuilder = builder;
        IRInst* subOutSpecialized = nullptr;
        auto genericFuncType = hoistValueFromGeneric(subBuilder, newResultVal->getFullType(), subOutSpecialized, false);
        newGeneric->setFullType((IRType*)genericFuncType);
    }
    else
    {
        newGeneric->setFullType(builder.getTypeKind());
    }
    if (replaceExistingValue)
    {
        builder.setInsertBefore(value);
        outSpecializedVal = specializeWithGeneric(builder, newGeneric, outerGeneric);
        value->replaceUsesWith(outSpecializedVal);
        value->removeAndDeallocate();
    }
    eliminateDeadCode(newGeneric);
    return newGeneric;
}

void moveInstChildren(IRInst* dest, IRInst* src)
{
    for (auto child = dest->getFirstDecorationOrChild(); child; )
    {
        auto next = child->getNextInst();
        child->removeAndDeallocate();
        child = next;
    }
    for (auto child = src->getFirstDecorationOrChild(); child; )
    {
        auto next = child->getNextInst();
        child->insertAtEnd(dest);
        child = next;
    }
}

}
