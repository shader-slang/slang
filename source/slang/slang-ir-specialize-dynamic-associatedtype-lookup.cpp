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
            builder.emitReturn(resultWitnessTableIDDecoration->getSequentialIDOperand());
        }

        // Emit a switch statement to return the correct witness table ID based on
        // the witness table ID passed in.
        builder.setInsertInto(func);
        auto breakBlock = builder.emitBlock();
        builder.setInsertInto(breakBlock);
        builder.emitUnreachable();

        builder.setInsertInto(block);
        builder.emitSwitch(
            witnessTableParam,
            breakBlock,
            defaultBlock,
            caseBlocks.getCount(),
            caseBlocks.getBuffer());

        return func;
    }

    // Retrieves the conformance type from a WitnessTableType or a WitnessTableIDType.
    IRInterfaceType* getInterfaceTypeFromWitnessTableTypes(IRInst* witnessTableType)
    {
        switch (witnessTableType->op)
        {
        case kIROp_WitnessTableType:
            return cast<IRInterfaceType>(
                cast<IRWitnessTableType>(witnessTableType)->getConformanceType());
        case kIROp_WitnessTableIDType:
            return cast<IRInterfaceType>(
                cast<IRWitnessTableIDType>(witnessTableType)->getConformanceType());
        default:
             return nullptr;
        }
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
        IRInterfaceType* interfaceType = getInterfaceTypeFromWitnessTableTypes(witnessTableType);
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
        if (witnessTableArg->getDataType()->op == kIROp_WitnessTableType)
        {
            witnessTableArg = builder.emitGetSequentialIDInst(witnessTableArg);
        }
        auto callInst = builder.emitCallInst(
            builder.getWitnessTableIDType(interfaceType), func, witnessTableArg);
        inst->replaceUsesWith(callInst);
        inst->removeAndDeallocate();
    }

    void cleanUpWitnessTableIDType()
    {
        List<IRInst*> instsToRemove;
        for (auto inst : sharedContext->module->getGlobalInsts())
        {
            if (inst->op == kIROp_WitnessTableIDType)
            {
                IRBuilder builder;
                builder.sharedBuilder = &sharedContext->sharedBuilderStorage;
                builder.setInsertBefore(inst);
                inst->replaceUsesWith(builder.getUIntType());
                instsToRemove.add(inst);
            }
        }
        for (auto inst : instsToRemove)
            inst->removeAndDeallocate();
    }

    void processGetSequentialIDInst(IRGetSequentialID* inst)
    {
        if (inst->getRTTIOperand()->getDataType()->op == kIROp_WitnessTableIDType)
        {
            inst->replaceUsesWith(inst->getRTTIOperand());
            inst->removeAndDeallocate();
        }
    }

    void processModule()
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

            if (inst->op == kIROp_lookup_interface_method)
            {
                processLookupInterfaceMethodInst(cast<IRLookupWitnessMethod>(inst));
            }

            for (auto child = inst->getLastChild(); child; child = child->getPrevInst())
            {
                sharedContext->addToWorkList(child);
            }
        }

        // `GetSequentialID(WitnessTableIDOperand)` becomes just `WitnessTableIDOperand`.
        sharedContext->addToWorkList(sharedContext->module->getModuleInst());
        while (sharedContext->workList.getCount() != 0)
        {
            IRInst* inst = sharedContext->workList.getLast();

            sharedContext->workList.removeLast();
            sharedContext->workListSet.Remove(inst);

            if (inst->op == kIROp_GetSequentialID)
            {
                processGetSequentialIDInst(cast<IRGetSequentialID>(inst));
            }

            for (auto child = inst->getLastChild(); child; child = child->getPrevInst())
            {
                sharedContext->addToWorkList(child);
            }
        }

        cleanUpWitnessTableIDType();
    }
};

void specializeDynamicAssociatedTypeLookup(SharedGenericsLoweringContext* sharedContext)
{
    AssociatedTypeLookupSpecializationContext context;
    context.sharedContext = sharedContext;
    context.processModule();
}

} // namespace Slang
