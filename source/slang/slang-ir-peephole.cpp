#include "slang-ir-peephole.h"
#include "slang-ir-inst-pass-base.h"

namespace Slang
{
struct PeepholeContext : InstPassBase
{
    PeepholeContext(IRModule* inModule)
        : InstPassBase(inModule)
    {}

    bool changed = false;

    void processInst(IRInst* inst)
    {
        switch (inst->getOp())
        {
        case kIROp_GetResultError:
            if (inst->getOperand(0)->getOp() == kIROp_MakeResultError)
            {
                inst->replaceUsesWith(inst->getOperand(0)->getOperand(0));
                changed = true;
            }
            break;
        case kIROp_GetResultValue:
            if (inst->getOperand(0)->getOp() == kIROp_MakeResultValue)
            {
                inst->replaceUsesWith(inst->getOperand(0)->getOperand(0));
                inst->removeAndDeallocate();
                changed = true;
            }
            break;
        case kIROp_IsResultError:
            if (inst->getOperand(0)->getOp() == kIROp_MakeResultError)
            {
                IRBuilder builder(&sharedBuilderStorage);
                inst->replaceUsesWith(builder.getBoolValue(true));
                inst->removeAndDeallocate();
                changed = true;
            }
            else if (inst->getOperand(0)->getOp() == kIROp_MakeResultValue)
            {
                IRBuilder builder(&sharedBuilderStorage);
                inst->replaceUsesWith(builder.getBoolValue(false));
                inst->removeAndDeallocate();
                changed = true;
            }
            break;
        case kIROp_GetTupleElement:
            if (inst->getOperand(0)->getOp() == kIROp_MakeTuple)
            {
                auto element = inst->getOperand(1);
                if (auto intLit = as<IRIntLit>(element))
                {
                    inst->replaceUsesWith(inst->getOperand(0)->getOperand((UInt)intLit->value.intVal));
                    inst->removeAndDeallocate();
                    changed = true;
                }
            }
            break;
        case kIROp_FieldExtract:
            if (inst->getOperand(0)->getOp() == kIROp_makeStruct)
            {
                auto field = as<IRFieldExtract>(inst)->field.get();
                Index fieldIndex = -1;
                auto structType = as<IRStructType>(inst->getOperand(0)->getDataType());
                if (structType)
                {
                    Index i = 0;
                    for (auto sfield : structType->getFields())
                    {
                        if (sfield == field)
                        {
                            fieldIndex = i;
                            break;
                        }
                        i++;
                    }
                    if (fieldIndex != -1 && fieldIndex < (Index)inst->getOperand(0)->getOperandCount())
                    {
                        inst->replaceUsesWith(inst->getOperand(0)->getOperand((UInt)fieldIndex));
                        inst->removeAndDeallocate();
                        changed = true;
                    }
                }
            }
            break;
        case kIROp_CastPtrToBool:
            {
                auto ptr = inst->getOperand(0);
                IRBuilder builder(&sharedBuilderStorage);
                builder.setInsertBefore(inst);
                auto neq = builder.emitNeq(ptr, builder.getPtrValue(nullptr));
                inst->replaceUsesWith(neq);
                inst->removeAndDeallocate();
                changed = true;
            }
            break;
        case kIROp_IsType:
            {
                auto isTypeInst = as<IRIsType>(inst);
                auto actualType = isTypeInst->getValue()->getDataType();
                if (isTypeEqual(actualType, (IRType*)isTypeInst->getTypeOperand()))
                {
                    IRBuilder builder(&sharedBuilderStorage);
                    builder.setInsertBefore(inst);
                    auto trueVal = builder.getBoolValue(true);
                    inst->replaceUsesWith(trueVal);
                    inst->removeAndDeallocate();
                    changed = true;
                }
            }
            break;
        case kIROp_Reinterpret:
            {
                if (isTypeEqual(inst->getOperand(0)->getDataType(), inst->getDataType()))
                {
                    inst->replaceUsesWith(inst->getOperand(0));
                    inst->removeAndDeallocate();
                    changed = true;
                }
            }
            break;
        case kIROp_UnpackAnyValue:
            {
                if (inst->getOperand(0)->getOp() == kIROp_PackAnyValue)
                {
                    if (isTypeEqual(inst->getOperand(0)->getOperand(0)->getDataType(), inst->getDataType()))
                    {
                        inst->replaceUsesWith(inst->getOperand(0)->getOperand(0));
                        inst->removeAndDeallocate();
                        changed = true;
                    }
                }
            }
            break;
        case kIROp_PackAnyValue:
        {
            // Pack(obj: anyValueN) : anyValueN --> obj
            if (isTypeEqual(inst->getOperand(0)->getDataType(), inst->getDataType()))
            {
                inst->replaceUsesWith(inst->getOperand(0));
                inst->removeAndDeallocate();
                changed = true;
            }
        }
        break;
        case kIROp_GetOptionalValue:
            {
                if (inst->getOperand(0)->getOp() == kIROp_MakeOptionalValue)
                {
                    inst->replaceUsesWith(inst->getOperand(0)->getOperand(0));
                    inst->removeAndDeallocate();
                    changed = true;
                }
            }
            break;
        case kIROp_OptionalHasValue:
            {
                if (inst->getOperand(0)->getOp() == kIROp_MakeOptionalValue)
                {
                    IRBuilder builder(&sharedBuilderStorage);
                    builder.setInsertBefore(inst);
                    auto trueVal = builder.getBoolValue(true);
                    inst->replaceUsesWith(trueVal);
                    inst->removeAndDeallocate();
                    changed = true;
                }
                else if (inst->getOperand(0)->getOp() == kIROp_MakeOptionalNone)
                {
                    IRBuilder builder(&sharedBuilderStorage);
                    builder.setInsertBefore(inst);
                    auto falseVal = builder.getBoolValue(false);
                    inst->replaceUsesWith(falseVal);
                    inst->removeAndDeallocate();
                    changed = true;
                }
            }
            break;
        default:
            break;
        }
    }

    bool processModule()
    {
        SharedIRBuilder* sharedBuilder = &sharedBuilderStorage;
        sharedBuilder->init(module);
        sharedBuilderStorage.deduplicateAndRebuildGlobalNumberingMap();

        changed = false;
        processAllInsts([this](IRInst* inst) { processInst(inst); });
        return changed;
    }
};

bool peepholeOptimize(IRModule* module)
{
    PeepholeContext context = PeepholeContext(module);
    return context.processModule();
}

} // namespace Slang
