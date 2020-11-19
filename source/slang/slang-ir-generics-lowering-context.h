// slang-ir-generics-lowering-context.h
#pragma once

#include "slang-ir.h"
#include "slang-ir-insts.h"

#include "slang-ir-lower-generics.h"

namespace Slang
{
    struct IRModule;

    constexpr IRIntegerValue kInvalidAnyValueSize = 0xFFFFFFFF;
    constexpr IRIntegerValue kDefaultAnyValueSize = 16;

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

        IRIntegerValue getInterfaceAnyValueSize(IRInst* type, SourceLoc usageLocation);
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

}
