// slang-ir-hlsl-util.cpp
#include "slang-ir-hlsl-util.h"

#include "slang-type-system-shared.h"

namespace Slang
{

bool isBarrierFlagGetterOp(IROp op)
{
    switch (op)
    {
    case kIROp_GetEnumBarrierMemoryTypeFlags:
    case kIROp_GetEnumBarrierSemanticFlags:
        return true;
    default:
        return false;
    }
}

IRInst* getBarrierFlagValueInst(IRInst* inst)
{
    while (inst->getOp() == kIROp_InOutImplicitCast || inst->getOp() == kIROp_OutImplicitCast)
        inst = inst->getOperand(0);
    return inst;
}

uint32_t getKnownBarrierMemoryTypeFlags()
{
    return BarrierMemoryTypeFlags::UavMemory | BarrierMemoryTypeFlags::GroupSharedMemory |
           BarrierMemoryTypeFlags::NodeInputMemory | BarrierMemoryTypeFlags::NodeOutputMemory;
}

uint32_t getKnownBarrierSemanticFlags()
{
    return BarrierSemanticFlags::GroupSync | BarrierSemanticFlags::GroupScope |
           BarrierSemanticFlags::DeviceScope;
}

bool isValidBarrierMemoryTypeFlags(uint32_t flagVal)
{
    auto knownFlags = getKnownBarrierMemoryTypeFlags();
    return flagVal == BarrierMemoryTypeFlags::AllMemory ||
           (flagVal != 0 && (flagVal & ~knownFlags) == 0);
}

bool isValidBarrierSemanticFlags(uint32_t flagVal)
{
    auto knownFlags = getKnownBarrierSemanticFlags();
    return flagVal == BarrierSemanticFlags::Reorder || (flagVal & ~knownFlags) == 0;
}

bool isBarrierFlagValueCast(IRInst* castInst, IRType* fromType, IRType* toType)
{
    SLANG_ASSERT(castInst);

    if (as<IRPtrTypeBase>(fromType) || as<IRPtrTypeBase>(toType))
        return false;

    bool hasUse = false;
    for (auto use = castInst->firstUse; use; use = use->nextUse)
    {
        hasUse = true;
        if (!isBarrierFlagGetterOp(use->getUser()->getOp()))
            return false;
    }
    return hasUse;
}

} // namespace Slang
