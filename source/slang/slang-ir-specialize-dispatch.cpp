#include "slang-ir-specialize-dispatch.h"

#include "slang-ir-generics-lowering-context.h"
#include "slang-ir-insts.h"
#include "slang-ir.h"

namespace Slang
{
IRFunc* specializeDispatchFunction(SharedGenericsLoweringContext* sharedContext, IRFunc* dispatchFunc)
{
    auto witnessTableType = cast<IRFuncType>(dispatchFunc->getDataType())->getParamType(0);
    auto conformanceType = cast<IRWitnessTableTypeBase>(witnessTableType)->getConformanceType();
    // Collect all witness tables of `witnessTableType` in current module.
    List<IRWitnessTable*> witnessTables = sharedContext->getWitnessTablesFromInterfaceType(conformanceType);

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
        switch (inst->getOp())
        {
        case kIROp_Call:
            callInst = cast<IRCall>(inst);
            break;
        case kIROp_lookup_interface_method:
            lookupInst = cast<IRLookupWitnessMethod>(inst);
            break;
        case kIROp_Return:
            returnInst = cast<IRReturn>(inst);
            break;
        default:
            break;
        }
    }
    SLANG_ASSERT(callInst && lookupInst && returnInst);

    IRBuilder builderStorage(sharedContext->sharedBuilderStorage);
    auto builder = &builderStorage;
    builder->setInsertBefore(dispatchFunc);

    // Create a new dispatch func to replace the existing one.
    auto newDispatchFunc = builder->createFunc();

    List<IRType*> paramTypes;
    for (auto paramInst : dispatchFunc->getParams())
    {
        paramTypes.add(paramInst->getFullType());
    }

    // Modify the first paramter from IRWitnessTable to IRWitnessTableID representing the sequential ID.
    paramTypes[0] = builder->getWitnessTableIDType((IRType*)conformanceType);

    auto newDipsatchFuncType = builder->getFuncType(paramTypes, dispatchFunc->getResultType());
    newDispatchFunc->setFullType(newDipsatchFuncType);
    dispatchFunc->transferDecorationsTo(newDispatchFunc);

    builder->setInsertInto(newDispatchFunc);
    auto newBlock = builder->emitBlock();

    IRBlock* defaultBlock = nullptr;

    auto requirementKey = lookupInst->getRequirementKey();
    List<IRInst*> params;
    for (Index i = 0; i < paramTypes.getCount(); i++)
    {
        auto param = builder->emitParam(paramTypes[i]);
        if (i > 0)
            params.add(param);
    }
    auto witnessTableParam = newBlock->getFirstParam();

    // `witnessTableParam` is expected to have `IRWitnessTableID` type, which
    // will later lower into a `uint2`. We only use the first element of the uint2
    // to store the sequential ID and reserve the second 32-bit value for future
    // pointer-compatibility. We insert a member extract inst right now
    // to obtain the first element and use it in our switch statement.
    UInt elemIdx = 0;
    auto witnessTableSequentialID =
        builder->emitSwizzle(builder->getUIntType(), witnessTableParam, 1, &elemIdx);

    // Generate case blocks for each possible witness table.
    List<IRInst*> caseBlocks;
    for (Index i = 0; i < witnessTables.getCount(); i++)
    {
        auto witnessTable = witnessTables[i];
        auto seqIdDecoration = witnessTable->findDecoration<IRSequentialIDDecoration>();
        SLANG_ASSERT(seqIdDecoration);
        
        if (i != witnessTables.getCount() - 1)
        {
            // Create a case block if we are not the last case.
            caseBlocks.add(seqIdDecoration->getSequentialIDOperand());
            builder->setInsertInto(newDispatchFunc);
            auto caseBlock = builder->emitBlock();
            caseBlocks.add(caseBlock);
        }
        else
        {
            // Generate code for the last possible value in the `default` block.
            builder->setInsertInto(newDispatchFunc);
            defaultBlock = builder->emitBlock();
            builder->setInsertInto(defaultBlock);
        }

        auto callee = sharedContext->findWitnessTableEntry(witnessTable, requirementKey);
        SLANG_ASSERT(callee);
        auto specializedCallInst = builder->emitCallInst(callInst->getFullType(), callee, params);
        if (callInst->getDataType()->getOp() == kIROp_VoidType)
            builder->emitReturn();
        else
            builder->emitReturn(specializedCallInst);
    }

    // Emit a switch statement to call the correct concrete function based on
    // the witness table sequential ID passed in.
    builder->setInsertInto(newDispatchFunc);


    if (witnessTables.getCount() == 1)
    {
        // If there is only 1 case, no switch statement is necessary.
        builder->setInsertInto(newBlock);
        builder->emitBranch(defaultBlock);
    }
    else if (witnessTables.getCount() > 1)
    {
        auto breakBlock = builder->emitBlock();
        builder->setInsertInto(breakBlock);
        builder->emitUnreachable();

        builder->setInsertInto(newBlock);
        builder->emitSwitch(
            witnessTableSequentialID,
            breakBlock,
            defaultBlock,
            caseBlocks.getCount(),
            caseBlocks.getBuffer());
    }
    else
    {
        // We have no witness tables that implements this interface.
        // Just return a default value.
        builder->setInsertInto(newBlock);
        if (callInst->getDataType()->getOp() == kIROp_VoidType)
        {
            builder->emitReturn();
        }
        else
        {
            auto defaultValue = builder->emitConstructorInst(callInst->getDataType(), 0, nullptr);
            builder->emitReturn(defaultValue);
        }
    }
    // Remove old implementation.
    dispatchFunc->replaceUsesWith(newDispatchFunc);
    dispatchFunc->removeAndDeallocate();

    return newDispatchFunc;
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
        if (inst->getOp() == kIROp_WitnessTable)
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
            IRBuilder builder(sharedContext->sharedBuilderStorage);
            builder.setInsertBefore(inst);
            builder.addSequentialIDDecoration(inst, seqID);
        }
    }
}

// Fixes up call sites of a dispatch function, so that the witness table argument is replaced with
// its sequential ID.
void fixupDispatchFuncCall(SharedGenericsLoweringContext* sharedContext, IRFunc* newDispatchFunc)
{
    List<IRInst*> users;
    for (auto use = newDispatchFunc->firstUse; use; use = use->nextUse)
    {
        users.add(use->getUser());
    }
    for (auto user : users)
    {
        if (auto call = as<IRCall>(user))
        {
            if (call->getCallee() != newDispatchFunc)
                continue;
            IRBuilder builder(sharedContext->sharedBuilderStorage);
            builder.setInsertBefore(call);
            List<IRInst*> args;
            for (UInt i = 0; i < call->getArgCount(); i++)
            {
                args.add(call->getArg(i));
            }
            if (as<IRWitnessTable>(args[0]->getDataType()))
                continue;
            auto newCall = builder.emitCallInst(call->getFullType(), newDispatchFunc, args);
            call->replaceUsesWith(newCall);
            call->removeAndDeallocate();
        }
    }
}

void specializeDispatchFunctions(SharedGenericsLoweringContext* sharedContext)
{
    sharedContext->sharedBuilderStorage.deduplicateAndRebuildGlobalNumberingMap();

    // First we ensure that all witness table objects has a sequential ID assigned.
    ensureWitnessTableSequentialIDs(sharedContext);

    // Generate specialized dispatch functions and fixup call sites.
    for (auto kv : sharedContext->mapInterfaceRequirementKeyToDispatchMethods)
    {
        auto dispatchFunc = kv.Value;

        // Generate a specialized `switch` statement based dispatch func,
        // from the witness tables present in the module.
        auto newDispatchFunc = specializeDispatchFunction(sharedContext, dispatchFunc);

        // Fix up the call sites of newDispatchFunc to pass in sequential IDs instead of
        // witness table objects.
        fixupDispatchFuncCall(sharedContext, newDispatchFunc);
    }
}
} // namespace Slang
