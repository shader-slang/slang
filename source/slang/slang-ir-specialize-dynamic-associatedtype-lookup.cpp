#include "slang-ir-specialize-dispatch.h"

#include "slang-ir-generics-lowering-context.h"
#include "slang-ir-insts.h"
#include "slang-ir.h"

namespace Slang
{

struct AssociatedTypeLookupSpecializationContext
{
    SharedGenericsLoweringContext* sharedContext;

    IRFunc* createWitnessTableLookupFunc(IRInterfaceType* interfaceType, IRInst* key)
    {
        IRBuilder builder;
        builder.sharedBuilder = &sharedContext->sharedBuilderStorage;
        builder.setInsertBefore(interfaceType);

        auto inputWitnessTableIDType = builder.getWitnessTableIDType(interfaceType);
        auto requirementEntry = sharedContext->findInterfaceRequirementVal(interfaceType, key);

        auto resultWitnessTableType = cast<IRWitnessTableType>(requirementEntry);
        auto resultWitnessTableIDType =
            builder.getWitnessTableIDType((IRType*)resultWitnessTableType->getConformanceType());

        auto funcType =
            builder.getFuncType(1, (IRType**)&inputWitnessTableIDType, resultWitnessTableIDType);
        auto func = builder.createFunc();
        func->setFullType(funcType);

        if (auto linkage = key->findDecoration<IRLinkageDecoration>())
            builder.addNameHintDecoration(func, linkage->getMangledName());

        builder.setInsertInto(func);

        auto block = builder.emitBlock();
        auto witnessTableParam = builder.emitParam(inputWitnessTableIDType);

        // `witnessTableParam` is expected to have `IRWitnessTableID` type, which
        // will later lower into a `uint2`. We only use the first element of the uint2
        // to store the sequential ID and reserve the second 32-bit value for future
        // pointer-compatibility. We insert a member extract inst right now
        // to obtain the first element and use it in our switch statement.
        UInt elemIdx = 0;
        auto witnessTableSequentialID =
            builder.emitSwizzle(builder.getUIntType(), witnessTableParam, 1, &elemIdx);

        // Collect all witness tables of `witnessTableType` in current module.
        List<IRWitnessTable*> witnessTables =
            sharedContext->getWitnessTablesFromInterfaceType(interfaceType);

        // Generate case blocks for each possible witness table.
        IRBlock* defaultBlock = nullptr;
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
                builder.setInsertInto(func);
                auto caseBlock = builder.emitBlock();
                caseBlocks.add(caseBlock);
            }
            else
            {
                // Generate code for the last possible value in the `default` block.
                builder.setInsertInto(func);
                defaultBlock = builder.emitBlock();
                builder.setInsertInto(defaultBlock);
            }

            auto resultWitnessTable = sharedContext->findWitnessTableEntry(witnessTable, key);
            auto resultWitnessTableIDDecoration =
                resultWitnessTable->findDecoration<IRSequentialIDDecoration>();
            SLANG_ASSERT(resultWitnessTableIDDecoration);
            // Pack the resulting witness table ID into a `uint2`.
            auto uint2Type = builder.getVectorType(
                builder.getUIntType(), builder.getIntValue(builder.getIntType(), 2));
            IRInst* uint2Args[] = {
                resultWitnessTableIDDecoration->getSequentialIDOperand(),
                builder.getIntValue(builder.getUIntType(), 0)};
            auto resultID = builder.emitMakeVector(uint2Type, 2, uint2Args);
            builder.emitReturn(resultID);
        }

        builder.setInsertInto(func);

        if (witnessTables.getCount() == 1)
        {
            // If there is only 1 case, no switch statement is necessary.
            builder.setInsertInto(block);
            builder.emitBranch(defaultBlock);
        }
        else
        {
            // If there are more than 1 cases,
            // emit a switch statement to return the correct witness table ID based on
            // the witness table ID passed in.
            auto breakBlock = builder.emitBlock();
            builder.setInsertInto(breakBlock);
            builder.emitUnreachable();

            builder.setInsertInto(block);
            builder.emitSwitch(
                witnessTableSequentialID,
                breakBlock,
                defaultBlock,
                caseBlocks.getCount(),
                caseBlocks.getBuffer());
        }

        return func;
    }

    void processLookupInterfaceMethodInst(IRLookupWitnessMethod* inst)
    {
        // Ignore lookups for RTTI objects for now, since they are not used anywhere.
        if (!as<IRWitnessTableType>(inst->getDataType()))
            return;

        // Replace all witness table lookups with calls to specialized functions that directly
        // returns the sequential ID of the resulting witness table, effectively getting rid
        // of actual witness table objects in the target code (they all become IDs).
        auto witnessTableType = inst->getWitnessTable()->getDataType();
        IRInterfaceType* interfaceType = cast<IRInterfaceType>(
            cast<IRWitnessTableTypeBase>(witnessTableType)->getConformanceType());
        if (!interfaceType)
            return;
        auto key = inst->getRequirementKey();
        IRFunc* func = nullptr;
        if (!sharedContext->mapInterfaceRequirementKeyToDispatchMethods.TryGetValue(key, func))
        {
            func = createWitnessTableLookupFunc(interfaceType, key);
            sharedContext->mapInterfaceRequirementKeyToDispatchMethods[key] = func;
        }
        IRBuilder builder;
        builder.sharedBuilder = &sharedContext->sharedBuilderStorage;
        builder.setInsertBefore(inst);
        auto witnessTableArg = inst->getWitnessTable();
        if (witnessTableArg->getDataType()->getOp() == kIROp_WitnessTableType)
        {
            witnessTableArg = builder.emitGetSequentialIDInst(witnessTableArg);
        }
        auto callInst = builder.emitCallInst(
            builder.getWitnessTableIDType(interfaceType), func, witnessTableArg);
        inst->replaceUsesWith(callInst);
        inst->removeAndDeallocate();
    }

    void processGetSequentialIDInst(IRGetSequentialID* inst)
    {
        if (inst->getRTTIOperand()->getDataType()->getOp() == kIROp_WitnessTableIDType)
        {
            inst->replaceUsesWith(inst->getRTTIOperand());
            inst->removeAndDeallocate();
        }
    }

    template<typename TFunc>
    void workOnModule(const TFunc& func)
    {
        SharedIRBuilder* sharedBuilder = &sharedContext->sharedBuilderStorage;
        sharedBuilder->module = sharedContext->module;
        sharedBuilder->session = sharedContext->module->session;

        sharedContext->addToWorkList(sharedContext->module->getModuleInst());

        while (sharedContext->workList.getCount() != 0)
        {
            IRInst* inst = sharedContext->workList.getLast();

            sharedContext->workList.removeLast();
            sharedContext->workListSet.Remove(inst);

            func(inst);
            if (inst->getOp() == kIROp_lookup_interface_method)
            {
                processLookupInterfaceMethodInst(cast<IRLookupWitnessMethod>(inst));
            }

            for (auto child = inst->getLastChild(); child; child = child->getPrevInst())
            {
                sharedContext->addToWorkList(child);
            }
        }
    }

    void processModule()
    {
        // Replace all `lookup_interface_method():IRWitnessTable` with call to specialized functions.
        workOnModule([this](IRInst* inst)
        {
            if (inst->getOp() == kIROp_lookup_interface_method)
            {
                processLookupInterfaceMethodInst(cast<IRLookupWitnessMethod>(inst));
            }
        });

        // Replace all direct uses of IRWitnessTables with its sequential ID.
        workOnModule([this](IRInst* inst)
        {
            if (inst->getOp() == kIROp_WitnessTable)
            {
                auto seqId = inst->findDecoration<IRSequentialIDDecoration>();
                SLANG_ASSERT(seqId);
                // Insert code to pack sequential ID into an uint2 at all use sites.
                for (auto use = inst->firstUse; use; )
                {
                    auto nextUse = use->nextUse;
                    IRBuilder builder;
                    builder.sharedBuilder = &sharedContext->sharedBuilderStorage;
                    builder.setInsertBefore(use->getUser());
                    auto uint2Type = builder.getVectorType(
                        builder.getUIntType(), builder.getIntValue(builder.getIntType(), 2));
                    IRInst* uint2Args[] = {
                        seqId->getSequentialIDOperand(),
                        builder.getIntValue(builder.getUIntType(), 0)};
                    auto uint2seqID = builder.emitMakeVector(uint2Type, 2, uint2Args);
                    use->set(uint2seqID);
                    use = nextUse;
                }
                inst->replaceUsesWith(seqId->getSequentialIDOperand());
            }
        });

        // Replace all `IRWitnessTableType`s with `IRWitnessTableIDType`.
        for (auto globalInst : sharedContext->module->getGlobalInsts())
        {
            if (globalInst->getOp() == kIROp_WitnessTableType)
            {
                IRBuilder builder;
                builder.sharedBuilder = &sharedContext->sharedBuilderStorage;
                builder.setInsertBefore(globalInst);
                auto witnessTableIDType = builder.getWitnessTableIDType(
                    (IRType*)cast<IRWitnessTableType>(globalInst)->getConformanceType());
                IRUse* nextUse = nullptr;
                for (auto use = globalInst->firstUse; use; use = nextUse)
                {
                    nextUse = use->nextUse;
                    if (use->getUser()->getOp() == kIROp_WitnessTable)
                        continue;
                    use->set(witnessTableIDType);
                }
            }
        }

        // `GetSequentialID(WitnessTableIDOperand)` becomes just `WitnessTableIDOperand`.
        workOnModule([this](IRInst* inst)
        {
            if (inst->getOp() == kIROp_GetSequentialID)
            {
                processGetSequentialIDInst(cast<IRGetSequentialID>(inst));
            }
        });
    }
};

void specializeDynamicAssociatedTypeLookup(SharedGenericsLoweringContext* sharedContext)
{
    AssociatedTypeLookupSpecializationContext context;
    context.sharedContext = sharedContext;
    context.processModule();
}

} // namespace Slang
