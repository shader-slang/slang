// slang-ir-generics-lowering-context.h
#pragma once

#include "slang-ir.h"
#include "slang-ir-insts.h"
#include "slang-ir-dce.h"

#include "slang-ir-lower-generics.h"

namespace Slang
{
    struct IRModule;

    constexpr IRIntegerValue kInvalidAnyValueSize = 0xFFFFFFFF;
    constexpr IRIntegerValue kDefaultAnyValueSize = 16;
    constexpr SlangInt kRTTIHeaderSize = 16;
    constexpr SlangInt kRTTIHandleSize = 8;

    struct SharedGenericsLoweringContext
    {
        // For convenience, we will keep a pointer to the module
        // we are processing.
        IRModule* module;

        TargetRequest* targetReq;

        DiagnosticSink* sink;

        // RTTI objects for each type used to call a generic function.
        OrderedDictionary<IRInst*, IRInst*> mapTypeToRTTIObject;

        Dictionary<IRInst*, IRInst*> loweredGenericFunctions;
        Dictionary<IRInterfaceType*, IRInterfaceType*> loweredInterfaceTypes;
        Dictionary<IRInterfaceType*, IRInterfaceType*> mapLoweredInterfaceToOriginal;


        // Dictionaries for interface type requirement key-value lookups.
        // Used by `findInterfaceRequirementVal`.
        Dictionary<IRInterfaceType*, Dictionary<IRInst*, IRInst*>> mapInterfaceRequirementKeyValue;
        
        // Map from interface requirement keys to its corresponding dispatch method.
        OrderedDictionary<IRInst*, IRFunc*> mapInterfaceRequirementKeyToDispatchMethods;

        SharedIRBuilder sharedBuilderStorage;

        // We will use a single work list of instructions that need
        // to be considered for lowering.
        //
        List<IRInst*> workList;
        HashSet<IRInst*> workListSet;

        void addToWorkList(
            IRInst* inst)
        {
            if (!inst) return;

            for (auto ii = inst->getParent(); ii; ii = ii->getParent())
            {
                if (as<IRGeneric>(ii))
                    return;
            }

            if (workListSet.Contains(inst))
                return;

            workList.add(inst);
            workListSet.Add(inst);
        }


        void _builldInterfaceRequirementMap(IRInterfaceType* interfaceType);

        IRInst* findInterfaceRequirementVal(IRInterfaceType* interfaceType, IRInst* requirementKey);

        // Emits an IRRTTIObject containing type information for a given type.
        IRInst* maybeEmitRTTIObject(IRInst* typeInst);

        static IRIntegerValue getInterfaceAnyValueSize(IRInst* type, SourceLoc usageLoc);
        IRType* lowerAssociatedType(IRBuilder* builder, IRInst* type);

        IRType* lowerType(IRBuilder* builder, IRInst* paramType, const Dictionary<IRInst*, IRInst*>& typeMapping, IRType* concreteType);

        IRType* lowerType(IRBuilder* builder, IRInst* paramType)
        {
            return lowerType(builder, paramType, Dictionary<IRInst*, IRInst*>(), nullptr);
        }

        // Get a list of all witness tables whose conformance type is `interfaceType`.
        List<IRWitnessTable*> getWitnessTablesFromInterfaceType(IRInst* interfaceType);

        IRInst* findWitnessTableEntry(IRWitnessTable* table, IRInst* key)
        {
            for (auto entry : table->getEntries())
            {
                if (entry->getRequirementKey() == key)
                    return entry->getSatisfyingVal();
            }
            return nullptr;
        }

            /// Does the given `concreteType` fit within the any-value size deterined by `interfaceType`?
        bool doesTypeFitInAnyValue(IRType* concreteType, IRInterfaceType* interfaceType);
    };

    bool isPolymorphicType(IRInst* typeInst);

    // Returns true if typeInst represents a type and should be lowered into
    // Ptr(RTTIType).
    bool isTypeValue(IRInst* typeInst);

    template<typename TFunc>
    void workOnModule(SharedGenericsLoweringContext* sharedContext, const TFunc& func)
    {
        SharedIRBuilder* sharedBuilder = &sharedContext->sharedBuilderStorage;
        sharedBuilder->init(sharedContext->module);

        sharedContext->addToWorkList(sharedContext->module->getModuleInst());

        while (sharedContext->workList.getCount() != 0)
        {
            IRInst* inst = sharedContext->workList.getLast();

            sharedContext->workList.removeLast();
            sharedContext->workListSet.Remove(inst);

            func(inst);

            for (auto child = inst->getLastChild(); child; child = child->getPrevInst())
            {
                sharedContext->addToWorkList(child);
            }
        }
    }

    template<typename TFunc>
    void workOnCallGraph(SharedGenericsLoweringContext* sharedContext, const TFunc& func)
    {
        SharedIRBuilder* sharedBuilder = &sharedContext->sharedBuilderStorage;
        sharedBuilder->init(sharedContext->module);

        sharedContext->addToWorkList(sharedContext->module->getModuleInst());
        IRDeadCodeEliminationOptions dceOptions;
        dceOptions.keepExportsAlive = true;
        dceOptions.keepLayoutsAlive = true;

        while (sharedContext->workList.getCount() != 0)
        {
            IRInst* inst = sharedContext->workList.getLast();

            sharedContext->workList.removeLast();

            sharedContext->addToWorkList(inst->parent);
            sharedContext->addToWorkList(inst->getFullType());

            UInt operandCount = inst->getOperandCount();
            for (UInt ii = 0; ii < operandCount; ++ii)
            {
                if (!isWeakReferenceOperand(inst, ii))
                    sharedContext->addToWorkList(inst->getOperand(ii));
            }

            if (auto call = as<IRCall>(inst))
            {
                if (func(call))
                    return;
            }

            for (auto child = inst->getLastChild(); child; child = child->getPrevInst())
            {
                if (shouldInstBeLiveIfParentIsLive(child, dceOptions))
                    sharedContext->addToWorkList(child);
            }
        }
    }
}
