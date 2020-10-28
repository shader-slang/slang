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

// Ensures every witness table object has been assigned a sequential ID.
// All witness tables will have a SequentialID decoration after this function is run.
// The sequantial ID in the decoration will be the same as the one specified in the Linkage.
// Otherwise, a new ID will be generated and assigned to the witness table object, and
// the sequantial ID map in the Linkage will be updated to include the new ID, so they
// can be looked up by the user via future Slang API calls.
void ensureWitnessTableSequentialIDs(SharedGenericsLoweringContext* sharedContext)
{
    auto linkage = sharedContext->targetReq->getLinkage();
    for (auto inst : sharedContext->module->getGlobalInsts())
    {
        if (inst->op == kIROp_WitnessTable)
        {
            UnownedStringSlice witnessTableMangledName;
            if (auto instLinkage = inst->findDecoration<IRLinkageDecoration>())
            {
                witnessTableMangledName = instLinkage->getMangledName();
            }
            else
            {
                // If this witness table entry does not have a linkage,
                // don't assign sequential ID for it.
                continue;
            }

            // If the inst already has a SequentialIDDecoration, stop now.
            if (inst->findDecoration<IRSequentialIDDecoration>())
                continue;

            // Get a sequential ID for the witness table using the map from the Linkage.
            uint32_t seqID = 0;
            if (!linkage->mapMangledNameToRTTIObjectIndex.TryGetValue(
                witnessTableMangledName, seqID))
            {
                auto interfaceType =
                    cast<IRWitnessTableType>(inst->getDataType())->getConformanceType();
                auto interfaceLinkage = interfaceType->findDecoration<IRLinkageDecoration>();
                SLANG_ASSERT(
                    interfaceLinkage && "An interface type does not have a linkage,"
                                        "but a witness table associated with it has one.");
                auto interfaceName = interfaceLinkage->getMangledName();
                auto idAllocator =
                    linkage->mapInterfaceMangledNameToSequentialIDCounters.TryGetValue(
                        interfaceName);
                if (!idAllocator)
                {
                    linkage->mapInterfaceMangledNameToSequentialIDCounters[interfaceName] = 0;
                    idAllocator =
                        linkage->mapInterfaceMangledNameToSequentialIDCounters.TryGetValue(
                            interfaceName);
                }
                seqID = *idAllocator;
                ++(*idAllocator);
                linkage->mapMangledNameToRTTIObjectIndex[witnessTableMangledName] = seqID;
            }

            // Add a decoration to the inst.
            IRBuilder builder;
            builder.sharedBuilder = &sharedContext->sharedBuilderStorage;
            builder.setInsertBefore(inst);
            builder.addSequentialIDDecoration(inst, seqID);
        }
    }
}

void specializeDispatchFunctions(SharedGenericsLoweringContext* sharedContext)
{
    sharedContext->sharedBuilderStorage.deduplicateAndRebuildGlobalNumberingMap();

    // First we ensure that all witness table objects has a sequential ID assigned.
    ensureWitnessTableSequentialIDs(sharedContext);

    for (auto kv : sharedContext->mapInterfaceRequirementKeyToDispatchMethods)
    {
        auto dispatchFunc = kv.Value;
        specializeDispatchFunction(sharedContext, dispatchFunc);
    }
}
} // namespace Slang
