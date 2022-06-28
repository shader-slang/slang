// slang-ir-lower-generics.cpp
#include "slang-ir-lower-generics.h"

#include "slang-ir-any-value-marshalling.h"
#include "slang-ir-augment-make-existential.h"
#include "slang-ir-generics-lowering-context.h"
#include "slang-ir-lower-existential.h"
#include "slang-ir-lower-generic-function.h"
#include "slang-ir-lower-generic-call.h"
#include "slang-ir-lower-generic-type.h"
#include "slang-ir-specialize-dispatch.h"
#include "slang-ir-specialize-dynamic-associatedtype-lookup.h"
#include "slang-ir-witness-table-wrapper.h"
#include "slang-ir-ssa.h"
#include "slang-ir-dce.h"

namespace Slang
{
    // Replace all uses of RTTI objects with its sequential ID.
    // Currently we don't use RTTI objects at all, so all of them
    // are 0.
    void specializeRTTIObjectReferences(SharedGenericsLoweringContext* sharedContext)
    {
        uint32_t id = 0;
        for (auto rtti : sharedContext->mapTypeToRTTIObject)
        {
            IRBuilder builder(sharedContext->sharedBuilderStorage);
            builder.setInsertBefore(rtti.Value);
            IRUse* nextUse = nullptr;
            auto uint2Type = builder.getVectorType(
                builder.getUIntType(), builder.getIntValue(builder.getIntType(), 2));
            IRInst* uint2Args[] = {
                builder.getIntValue(builder.getUIntType(), id),
                builder.getIntValue(builder.getUIntType(), 0)};
            auto idOperand = builder.emitMakeVector(uint2Type, 2, uint2Args);
            for (auto use = rtti.Value->firstUse; use; use = nextUse)
            {
                nextUse = use->nextUse;
                if (use->getUser()->getOp() == kIROp_getAddr)
                {
                    use->getUser()->replaceUsesWith(idOperand);
                }
            }
        }
    }

    // Replace all WitnessTableID type or RTTIHandleType with `uint2`.
    void cleanUpRTTIHandleTypes(SharedGenericsLoweringContext* sharedContext)
    {
        List<IRInst*> instsToRemove;
        for (auto inst : sharedContext->module->getGlobalInsts())
        {
            switch (inst->getOp())
            {
            case kIROp_WitnessTableIDType:
            case kIROp_RTTIHandleType:
                {
                    IRBuilder builder(sharedContext->sharedBuilderStorage);
                    builder.setInsertBefore(inst);
                    auto uint2Type = builder.getVectorType(
                        builder.getUIntType(), builder.getIntValue(builder.getIntType(), 2));
                    inst->replaceUsesWith(uint2Type);
                    instsToRemove.add(inst);
                }
                break;
            }
        }
        for (auto inst : instsToRemove)
            inst->removeAndDeallocate();
    }

    // Remove all interface types from module.
    void cleanUpInterfaceTypes(SharedGenericsLoweringContext* sharedContext)
    {
        IRBuilder builder(sharedContext->sharedBuilderStorage);
        builder.setInsertInto(sharedContext->module->getModuleInst());
        auto dummyInterfaceObj = builder.getIntValue(builder.getIntType(), 0);
        List<IRInst*> interfaceInsts;
        for (auto inst : sharedContext->module->getGlobalInsts())
        {
            if (inst->getOp() == kIROp_InterfaceType)
            {
                if (inst->findDecoration<IRComInterfaceDecoration>())
                    continue;

                interfaceInsts.add(inst);
            }
        }
        for (auto inst : interfaceInsts)
        {
            inst->replaceUsesWith(dummyInterfaceObj);
            inst->removeAndDeallocate();
        }
    }

    // Turn all references of witness table or RTTI objects into integer IDs, generate
    // specialized `switch` based dispatch functions based on witness table IDs, and remove
    // all original witness table, RTTI object and interface definitions from IR module.
    // With these transformations, the resulting code is compatible with D3D/Vulkan where
    // no pointers are involved in RTTI / dynamic dispatch logic.
    void specializeRTTIObjects(SharedGenericsLoweringContext* sharedContext, DiagnosticSink* sink)
    {
        specializeDispatchFunctions(sharedContext);
        if (sink->getErrorCount() != 0)
            return;

        specializeDynamicAssociatedTypeLookup(sharedContext);
        if (sink->getErrorCount() != 0)
            return;

        sharedContext->sharedBuilderStorage.deduplicateAndRebuildGlobalNumberingMap();
        sharedContext->mapInterfaceRequirementKeyValue.Clear();

        specializeRTTIObjectReferences(sharedContext);

        cleanUpRTTIHandleTypes(sharedContext);

        cleanUpInterfaceTypes(sharedContext);
    }

    void checkTypeConformanceExists(SharedGenericsLoweringContext* context)
    {
        HashSet<IRInst*> implementedInterfaces;

        // Add all interface type that are implemented by at least one type to a set.
        for (auto inst : context->module->getGlobalInsts())
        {
            if (inst->getOp() == kIROp_WitnessTable)
            {
                auto interfaceType = cast<IRWitnessTableType>(inst->getDataType())->getConformanceType();
                implementedInterfaces.Add(interfaceType);
            }
        }
        // Check if an interface type has any implementations.
        workOnModule(context, [&](IRInst* inst)
            {
                if (auto lookupWitnessMethod = as<IRLookupWitnessMethod>(inst))
                {
                    auto witnessTableType = lookupWitnessMethod->getWitnessTable()->getDataType();
                    if (!witnessTableType)
                        return;
                    auto interfaceType = cast<IRWitnessTableType>(witnessTableType)->getConformanceType();
                    if (isComInterfaceType((IRType*)interfaceType))
                        return;
                    if (!implementedInterfaces.Contains(interfaceType))
                    {
                        context->sink->diagnose(interfaceType->sourceLoc, Diagnostics::noTypeConformancesFoundForInterface, interfaceType);
                        // Add to set to prevent duplicate diagnostic messages.
                        implementedInterfaces.Add(interfaceType);
                    }
                }
            });
    }

    void lowerGenerics(
        TargetRequest*          targetReq,
        IRModule*               module,
        DiagnosticSink*         sink)
    {
        SharedGenericsLoweringContext sharedContext;
        sharedContext.targetReq = targetReq;
        sharedContext.module = module;
        sharedContext.sink = sink;

        checkTypeConformanceExists(&sharedContext);

        // Replace all `makeExistential` insts with `makeExistentialWithRTTI`
        // before making any other changes. This is necessary because a parameter of
        // generic type will be lowered into `AnyValueType`, and after that we can no longer
        // access the original generic type parameter from the lowered parameter value.
        // This steps ensures that the generic type parameter is available via an
        // explicit operand in `makeExistentialWithRTTI`, so that type parameter
        // can be translated into an RTTI object during `lower-generic-type`,
        // and used to create a tuple representing the existential value.
        augmentMakeExistentialInsts(module);

        lowerGenericFunctions(&sharedContext);
        if (sink->getErrorCount() != 0)
            return;

        lowerGenericType(&sharedContext);
        if (sink->getErrorCount() != 0)
            return;

        lowerExistentials(&sharedContext);
        if (sink->getErrorCount() != 0)
            return;

        lowerGenericCalls(&sharedContext);
        if (sink->getErrorCount() != 0)
            return;

        generateWitnessTableWrapperFunctions(&sharedContext);
        if (sink->getErrorCount() != 0)
            return;

        generateAnyValueMarshallingFunctions(&sharedContext);
        if (sink->getErrorCount() != 0)
            return;

        // This optional step replaces all uses of witness tables and RTTI objects with
        // sequential IDs. Without this step, we will emit code that uses function pointers and
        // real RTTI objects and witness tables.
        specializeRTTIObjects(&sharedContext, sink);

        // We might have generated new temporary variables during lowering.
        // An SSA pass can clean up unnecessary load/stores.
        constructSSA(module);
        eliminateDeadCode(module);
    }
} // namespace Slang
