#include "slang-ir-util.h"
#include "slang-ir-insts.h"

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

}
