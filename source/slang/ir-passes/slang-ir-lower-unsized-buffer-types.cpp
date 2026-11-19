// slang-ir-lower-unsized-buffer-types.cpp

#include "slang-ir-lower-unsized-buffer-types.h"

#include "slang-ir-inst-pass-base.h"
#include "slang/ir/slang-ir.h"

namespace Slang
{

void lowerUnsizedBufferTypesToResources(IRModule* module)
{
    IRBuilder builder(module);
    InstPassBase pass(module);
    pass.processAllInsts(
        [&](IRInst* inst)
        {
            IROp resourceOp;
            switch (inst->getOp())
            {
            case kIROp_HLSLUnsizedStructuredBufferType:
                resourceOp = kIROp_HLSLStructuredBufferType;
                break;
            case kIROp_HLSLUnsizedRWStructuredBufferType:
                resourceOp = kIROp_HLSLRWStructuredBufferType;
                break;
            case kIROp_HLSLUnsizedByteAddressBufferType:
                resourceOp = kIROp_HLSLByteAddressBufferType;
                break;
            case kIROp_HLSLUnsizedRWByteAddressBufferType:
                resourceOp = kIROp_HLSLRWByteAddressBufferType;
                break;
            default:
                return;
            }
            ShortList<IRInst*> operands;
            for (UInt i = 0; i < inst->getOperandCount(); ++i)
                operands.add(inst->getOperand(i));
            builder.setInsertBefore(inst);
            auto resourceType = builder.getType(
                resourceOp,
                operands.getCount(),
                operands.getArrayView().getBuffer());
            inst->replaceUsesWith(resourceType);
        });
    pass.processAllInsts(
        [&](IRInst* inst)
        {
            if (inst->getOp() == kIROp_GetUnsizedBuffer)
            {
                SLANG_ASSERT(inst->getDataType() == inst->getOperand(0)->getDataType());
                inst->replaceUsesWith(inst->getOperand(0));
                inst->removeAndDeallocate();
            }
        });
}

} // namespace Slang
