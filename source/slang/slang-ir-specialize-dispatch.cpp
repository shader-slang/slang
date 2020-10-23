#include "slang-ir-specialize-dispatch.h"

#include "slang-ir-generics-lowering-context.h"
#include "slang-ir-insts.h"
#include "slang-ir.h"

namespace Slang
{
IRInst* findWitnessTableEntry(IRWitnessTable* table, IRInst* key)
{
    for (auto entry : table->getEntries())
    {
        if (entry->getRequirementKey() == key)
            return entry->getSatisfyingVal();
    }
    return nullptr;
}

void specializeDispatchFunction(SharedGenericsLoweringContext* sharedContext, IRFunc* dispatchFunc)
{
    auto witnessTableType = cast<IRFuncType>(dispatchFunc->getDataType())->getParamType(0);

    // Collect all witness tables of `witnessTableType` in current module.
    List<IRWitnessTable*> witnessTables;
    for (auto globalInst : sharedContext->module->getGlobalInsts())
    {
        if (globalInst->op == kIROp_WitnessTable && globalInst->getDataType() == witnessTableType)
        {
            witnessTables.add(cast<IRWitnessTable>(globalInst));
        }
    }

    SLANG_ASSERT(dispatchFunc->getFirstBlock() == dispatchFunc->getLastBlock());
    auto block = dispatchFunc->getFirstBlock();

    // The dispatch function before modification must be in the form of
    // call(lookup_interface_method(witnessTableParam, interfaceReqKey), args)
    // We now find the relavent instructions.
    IRCall* callInst = nullptr;
    IRLookupWitnessMethod* lookupInst = nullptr;
    IRReturn* returnInst = nullptr;
    for (auto inst : block->getOrdinaryInsts())
    {
        switch (inst->op)
        {
        case kIROp_Call:
            callInst = cast<IRCall>(inst);
            break;
        case kIROp_lookup_interface_method:
            lookupInst = cast<IRLookupWitnessMethod>(inst);
            break;
        case kIROp_ReturnVal:
        case kIROp_ReturnVoid:
            returnInst = cast<IRReturn>(inst);
            break;
        default:
            break;
        }
    }
    SLANG_ASSERT(callInst && lookupInst && returnInst);

    IRBuilder builderStorage;
    auto builder = &builderStorage;
    builder->sharedBuilder = &sharedContext->sharedBuilderStorage;
    builder->setInsertBefore(callInst);

    auto witnessTableParam = block->getFirstParam();
    auto requirementKey = lookupInst->getRequirementKey();
    List<IRInst*> params;
    for (auto param = block->getFirstParam()->getNextParam(); param; param = param->getNextParam())
    {
        params.add(param);
    }

    // Emit cascaded if statements to call the correct concrete function based on
    // the witness table pointer passed in.
    auto ifBlock = block;
    for (Index i = 0; i < witnessTables.getCount(); i++)
    {
        auto witnessTable = witnessTables[i];
        bool isLast = (i == witnessTables.getCount() - 1);
        IRInst* cmpArgs[] =
        {
            builder->emitBitCast(builder->getUInt64Type(), witnessTableParam),
            builder->emitBitCast(builder->getUInt64Type(),(IRInst*)witnessTable)
        };
        IRInst* condition = nullptr;
        IRBlock* trueBlock = nullptr;
        if (!isLast)
        {
            condition = builder->emitIntrinsicInst(builder->getBoolType(), kIROp_Eql, 2, cmpArgs);
            trueBlock = builder->emitBlock();
        }
        auto callee = findWitnessTableEntry(witnessTable, requirementKey);
        SLANG_ASSERT(callee);
        auto specializedCallInst = builder->emitCallInst(callInst->getFullType(), callee, params);
        if (callInst->getDataType()->op == kIROp_VoidType)
            builder->emitReturn();
        else
            builder->emitReturn(specializedCallInst);
        if (!isLast)
        {
            auto falseBlock = builder->emitBlock();
            builder->setInsertInto(ifBlock);
            builder->emitIf(condition, trueBlock, falseBlock);
            builder->setInsertInto(falseBlock);
            ifBlock = falseBlock;
        }
    }

    // Remove old implementation.
    lookupInst->removeAndDeallocate();
    callInst->removeAndDeallocate();
    returnInst->removeAndDeallocate();
}

void specializeDispatchFunctions(SharedGenericsLoweringContext* sharedContext)
{
    sharedContext->sharedBuilderStorage.deduplicateAndRebuildGlobalNumberingMap();

    for (auto kv : sharedContext->mapInterfaceRequirementKeyToDispatchMethods)
    {
        auto dispatchFunc = kv.Value;
        specializeDispatchFunction(sharedContext, dispatchFunc);
    }
}
} // namespace Slang
