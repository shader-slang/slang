// slang-ir-lower-witness-lookup-chain.cpp
#include "slang-ir-clone.h"
#include "slang-ir-generics-lowering-context.h"
#include "slang-ir-insts.h"
#include "slang-ir-lower-witness-lookup.h"
#include "slang-ir-util.h"
#include "slang-ir.h"

namespace Slang
{

// TODO: Move to utils
static List<IRWitnessTable*> getWitnessTablesFromInterfaceType(
    IRModule* module,
    IRInst* interfaceType)
{
    List<IRWitnessTable*> witnessTables;
    for (auto globalInst : module->getGlobalInsts())
    {
        if (globalInst->getOp() == kIROp_WitnessTable &&
            cast<IRWitnessTableType>(globalInst->getDataType())->getConformanceType() ==
                interfaceType)
        {
            witnessTables.add(cast<IRWitnessTable>(globalInst));
        }
    }
    return witnessTables;
}

struct WitnessChainLookupLoweringContext
{
    IRModule* module;
    DiagnosticSink* sink;

    Dictionary<IRStructKey*, IRInst*> witnessDispatchFunctions;

    void init()
    {
        // Reconstruct the witness dispatch functions map.
        for (auto inst : module->getGlobalInsts())
        {
            if (auto key = as<IRStructKey>(inst))
            {
                for (auto decor : key->getDecorations())
                {
                    if (auto witnessDispatchFunc = as<IRDispatchFuncDecoration>(decor))
                    {
                        witnessDispatchFunctions.add(key, witnessDispatchFunc->getFunc());
                    }
                }
            }
        }
    }

    bool hasAssocType(IRInst* type)
    {
        if (!type)
            return false;

        InstHashSet processedSet(type->getModule());
        InstWorkList workList(type->getModule());
        workList.add(type);
        processedSet.add(type);
        for (Index i = 0; i < workList.getCount(); i++)
        {
            auto inst = workList[i];
            if (inst->getOp() == kIROp_AssociatedType)
                return true;

            for (UInt j = 0; j < inst->getOperandCount(); j++)
            {
                if (!inst->getOperand(j))
                    continue;
                if (processedSet.add(inst->getOperand(j)))
                    workList.add(inst->getOperand(j));
            }
        }
        return false;
    }

    IRType* translateType(IRBuilder builder, IRInst* type)
    {
        if (!type)
            return nullptr;
        if (auto genType = as<IRGeneric>(type))
        {
            IRCloneEnv cloneEnv;
            builder.setInsertBefore(genType);
            auto newGeneric = as<IRGeneric>(cloneInst(&cloneEnv, &builder, genType));
            newGeneric->setFullType(builder.getGenericKind());
            auto retVal = findGenericReturnVal(newGeneric);
            builder.setInsertBefore(retVal);
            auto translated = translateType(builder, retVal);
            retVal->replaceUsesWith(translated);
            return (IRType*)newGeneric;
        }
        else if (auto thisType = as<IRThisType>(type))
        {
            return (IRType*)thisType->getConstraintType();
        }
        else if (auto assocType = as<IRAssociatedType>(type))
        {
            return assocType;
        }

        if (as<IRBasicType>(type))
            return (IRType*)type;

        switch (type->getOp())
        {
        case kIROp_Param:
        case kIROp_VectorType:
        case kIROp_MatrixType:
        case kIROp_StructType:
        case kIROp_ClassType:
        case kIROp_InterfaceType:
            return (IRType*)type;
        default:
            {
                List<IRInst*> translatedOperands;
                for (UInt i = 0; i < type->getOperandCount(); i++)
                {
                    translatedOperands.add(translateType(builder, type->getOperand(i)));
                }
                auto translated = builder.emitIntrinsicInst(
                    type->getFullType(),
                    type->getOp(),
                    (UInt)translatedOperands.getCount(),
                    translatedOperands.getBuffer());
                return (IRType*)translated;
            }
        }
    }

    struct ExistentialBaseResult
    {
        IRInst* baseInst;
        enum Flavor
        {
            One,
            Many,
            None
        };

        Flavor flavor;

        void combine(const ExistentialBaseResult& other)
        {
            if (flavor == None)
            {
                baseInst = other.baseInst;
                flavor = other.flavor;
            }
            else if (other.flavor == None)
            {
                return;
            }
            else if (flavor == One && other.flavor == One)
            {
                if (baseInst != other.baseInst)
                    flavor = Many;
            }
            else
            {
                flavor = Many;
            }
        }
    };

    IRFunc* getParentFunc(IRInst* inst)
    {
        // Get the parent function of the inst.
        auto parentFunc = inst->getParent();
        while (parentFunc && !as<IRFunc>(parentFunc))
        {
            parentFunc = parentFunc->getParent();
        }
        return as<IRFunc>(parentFunc);
    }

    ExistentialBaseResult getUniqueExistentialBaseInst(IRFunc* parentFunc, IRInst* inst)
    {
        // Recursively get a unique base inst from the inst,
        // its operands until we hit an ExtractExistentialType
        // inst or leave the parent function.
        //

        if (getParentFunc(inst) != parentFunc)
            return {nullptr, ExistentialBaseResult::None};

        if (auto extractInst = as<IRExtractExistentialType>(inst))
        {
            return {extractInst->getOperand(0), ExistentialBaseResult::One};
        }
        else if (auto extractVal = as<IRExtractExistentialValue>(inst))
        {
            return {extractVal->getOperand(0), ExistentialBaseResult::One};
        }
        else if (auto witnessTableInst = as<IRExtractExistentialWitnessTable>(inst))
        {
            return {witnessTableInst->getOperand(0), ExistentialBaseResult::One};
        }
        else
        {
            // General inst.. combine the results of all operands and the type.
            ExistentialBaseResult result = {nullptr, ExistentialBaseResult::None};
            for (UInt i = 0; i < inst->getOperandCount(); i++)
            {
                auto operandResult = getUniqueExistentialBaseInst(parentFunc, inst->getOperand(i));
                result.combine(operandResult);
            }
            return result;
        }
    }

    struct LookupChain
    {
        IRInterfaceType* baseType;
        ShortList<IRStructKey*> keys;

        bool operator==(const LookupChain& other) const
        {
            if (baseType != other.baseType)
                return false;
            if (keys.getCount() != other.keys.getCount())
                return false;
            for (Index i = 0; i < keys.getCount(); i++)
            {
                if (keys[i] != other.keys[i])
                    return false;
            }
            return true;
        }

        LookupChain()
            : baseType(nullptr), keys(ShortList<IRStructKey*>())
        {
        }

        LookupChain(IRInterfaceType* baseType)
            : baseType(baseType), keys(ShortList<IRStructKey*>())
        {
        }

        // getHashCode
        HashCode64 getHashCode() const
        {
            HashCode64 hash = ::Slang::getHashCode(baseType);
            for (auto key : keys)
                hash = combineHash(hash, ::Slang::getHashCode(key));
            return combineHash(hash, ::Slang::getHashCode(baseType));
        }

        bool isTrivial() const { return keys.getCount() == 1; }
    };

    struct InterfaceDef
    {
        // Keep track of the requirements that are being folded into the
        // new interface type.
        //
        // (We'll turn it into an IRInterfaceType later, since we need to know
        // how many requirements we have before we can create the new type.)
        //
        Dictionary<IRStructKey*, IRInst*> newRequirements;
        IRInterfaceType* base;

        // Constructor
        InterfaceDef() { base = nullptr; }
        InterfaceDef(IRBuilder* builder) { base = builder->createInterfaceType(0, nullptr); }
        InterfaceDef(IRInterfaceType* base)
            : base(base)
        {
        }
    };

    Dictionary<LookupChain, IRStructKey*> keys;
    Dictionary<IRStructKey*, LookupChain> chains;
    Dictionary<LookupChain, InterfaceDef> assocTypeInterfaces;
    Dictionary<IRInterfaceType*, InterfaceDef> interfaceExtensionDefs;

    IRStructKey* compressChain(LookupChain& chain, IRInst* effectiveType)
    {
        // get or create a struct key, then insert it into the chain
        IRStructKey* key = getOrCreateKey(chain);

        // get or create an interface-def for the base interface.
        auto baseInterfaceType = chain.baseType;

        // If we don't have an interface def for the base interface, create one.
        interfaceExtensionDefs.addIfNotExists(baseInterfaceType, InterfaceDef(baseInterfaceType));
        interfaceExtensionDefs[baseInterfaceType].newRequirements.addIfNotExists(
            key,
            effectiveType);
        if (interfaceExtensionDefs[baseInterfaceType].newRequirements[key] != effectiveType)
        {
            SLANG_UNEXPECTED("Conflicting effective types for the same key.");
        }
        return key;
    }

    IRStructKey* getOrCreateKey(LookupChain& chain)
    {
        IRStructKey* key = nullptr;
        if (!keys.tryGetValue(chain, key))
        {
            // Create a new key.
            auto builder = IRBuilder(module);
            key = builder.createStructKey();
            keys.add(chain, key);
            chains.add(key, chain);
        }
        return key;
    }

    LookupChain getLookupChain(IRLookupWitnessMethod* lookupInst)
    {
        LookupChain chain;
        // Track the lookup back to the base type
        auto witnessTable = lookupInst->getWitnessTable();
        chain.keys.add(cast<IRStructKey>(lookupInst->getRequirementKey()));

        while (as<IRLookupWitnessMethod>(witnessTable))
        {
            auto _lookupInst = as<IRLookupWitnessMethod>(witnessTable);
            chain.keys.add(cast<IRStructKey>(_lookupInst->getRequirementKey()));
            witnessTable = _lookupInst->getWitnessTable();
        }

        auto extractInst = as<IRExtractExistentialWitnessTable>(witnessTable);
        SLANG_ASSERT(extractInst);
        auto interfaceType =
            as<IRWitnessTableTypeBase>(extractInst->getDataType())->getConformanceType();
        SLANG_ASSERT(interfaceType);
        auto baseType = as<IRInterfaceType>(unwrapAttributedType(interfaceType));
        SLANG_ASSERT(baseType);

        chain.baseType = baseType;

        return chain;
    }

    IRInst* getLookupBaseInst(IRLookupWitnessMethod* lookupInst)
    {
        // Track the lookup back to the base type
        auto witnessTable = lookupInst->getWitnessTable();

        while (as<IRLookupWitnessMethod>(witnessTable))
        {
            auto _lookupInst = as<IRLookupWitnessMethod>(witnessTable);
            witnessTable = _lookupInst->getWitnessTable();
        }

        auto extractInst = as<IRExtractExistentialWitnessTable>(witnessTable);
        SLANG_ASSERT(extractInst);

        return extractInst->getOperand(0);
    }

    // Process an associated type lookup
    IRInst* translateTypeLookup(IRBuilder* builder, IRLookupWitnessMethod* lookupInst)
    {
        LookupChain chain = getLookupChain(lookupInst);
        if (chain.isTrivial())
            return lookupInst;

        assocTypeInterfaces.addIfNotExists(chain, InterfaceDef(builder));

        auto effectiveAssocType = builder->getAssociatedType(
            List<IRInterfaceType*>(assocTypeInterfaces[chain].base).getArrayView());

        // Compresses the chain, registers it for lowering, and
        // returns an effective key.
        //
        auto key = compressChain(chain, effectiveAssocType);

        return builder->emitLookupInterfaceMethodInst(
            builder->getTypeKind(),
            builder->emitExtractExistentialWitnessTable(getLookupBaseInst(lookupInst)),
            key);
    }

    // Translate a type using in a function to an interface requirement type.
    IRInst* translateTypeToRequirementType(IRBuilder* builder, IRInst* type)
    {
        switch (type->getOp())
        {
        case kIROp_LookupWitness:
            {
                auto witnessTable = as<IRLookupWitnessMethod>(type)->getWitnessTable();
                if (auto extractTable = as<IRExtractExistentialWitnessTable>(witnessTable))
                {
                    auto interfaceType = cast<IRInterfaceType>(
                        as<IRWitnessTableType>(extractTable->getDataType())->getConformanceType());
                    auto reqType = findInterfaceRequirement(
                        cast<IRInterfaceType>(interfaceType),
                        as<IRLookupWitnessMethod>(type)->getRequirementKey());
                    if (!reqType && interfaceExtensionDefs.containsKey(interfaceType))
                    {
                        // Look in the interface extension definitions
                        auto& interfaceDef = interfaceExtensionDefs[interfaceType];
                        reqType = interfaceDef.newRequirements[as<IRStructKey>(
                            as<IRLookupWitnessMethod>(type)->getRequirementKey())];
                    }

                    SLANG_ASSERT(reqType);
                    return reqType;
                }
                else
                {
                    SLANG_UNEXPECTED("Should not see this case.");
                }
                break;
            }
        case kIROp_ExtractExistentialType:
            {
                auto extractType = as<IRExtractExistentialType>(type);
                auto interfaceType =
                    cast<IRInterfaceType>(extractType->getOperand(0)->getDataType());
                return builder->getThisType(interfaceType);
            }
        default:
            {
                if (type->getParent() == type->getModule()->getModuleInst())
                {
                    return type;
                }
                else
                {
                    SLANG_UNEXPECTED("Unhandled type");
                }
            }
        }
    }

    // Translate a func-type at the call-site to a func-type that
    // should be used for the interface requirement.
    //
    IRFuncType* translateFuncTypeToRequirementType(IRFuncType* funcType)
    {
        IRBuilder builder(funcType->getModule());
        // ExtractExistentialType -> ThisType
        // LookupWitness(...) -> AssociatedType(AssocTypeInterface)
        //
        List<IRType*> translatedTypes;
        for (auto paramType : funcType->getParamTypes())
        {
            translatedTypes.add((IRType*)translateTypeToRequirementType(&builder, paramType));
        }

        auto resultType =
            (IRType*)translateTypeToRequirementType(&builder, funcType->getResultType());

        return builder.getFuncType(translatedTypes, resultType);
    }

    // Process a witness method lookup
    IRInst* translateMethodLookup(IRBuilder* builder, IRLookupWitnessMethod* lookupInst)
    {
        auto chain = getLookupChain(lookupInst);

        if (chain.isTrivial())
            return lookupInst;

        // Translate func-type appropriately
        // (lookups -> assoc-type,
        // extract-type -> this-type)
        //
        auto funcType = cast<IRFuncType>(lookupInst->getDataType());

        List<IRType*> compressedParamTypes;
        for (auto paramType : funcType->getParamTypes())
        {
            switch (paramType->getOp())
            {
            case kIROp_LookupWitness:
                {
                    compressedParamTypes.add((
                        IRType*)translateTypeLookup(builder, as<IRLookupWitnessMethod>(paramType)));
                    break;
                }
            default:
                {
                    compressedParamTypes.add(paramType);
                    break;
                }
            }
        }

        auto resultType = funcType->getResultType();
        if (resultType->getOp() == kIROp_LookupWitness)
        {
            resultType =
                (IRType*)translateTypeLookup(builder, as<IRLookupWitnessMethod>(resultType));
        }

        auto compressedFuncType = builder->getFuncType(compressedParamTypes, resultType);

        auto reqType = translateFuncTypeToRequirementType(compressedFuncType);

        auto key = compressChain(chain, reqType);

        return builder->emitLookupInterfaceMethodInst(
            compressedFuncType,
            builder->emitExtractExistentialWitnessTable(getLookupBaseInst(lookupInst)),
            key);
    }

    // Process a witness table lookup
    IRInst* translateTableLookup(IRBuilder* builder, IRLookupWitnessMethod* lookupInst)
    {
        SLANG_UNIMPLEMENTED_X("Shouldn't see this case just yet");
    }

    IRInst* translateLookup(IRBuilder* builder, IRLookupWitnessMethod* lookupInst)
    {
        if (as<IRTypeKind>(lookupInst->getDataType()))
        {
            return translateTypeLookup(builder, lookupInst);
        }
        else if (as<IRWitnessTableTypeBase>(lookupInst->getDataType()))
        {
            return translateTableLookup(builder, lookupInst);
        }
        else if (as<IRFuncType>(lookupInst->getDataType()))
        {
            return translateMethodLookup(builder, lookupInst);
        }
        else
        {
            SLANG_UNEXPECTED("Unhandled lookup type");
        }
    }

    bool processExistentialCall(IRCall* callInst)
    {
        auto parentFunc = getParentFunc(callInst);
        if (!parentFunc)
            return false;

        IRLookupWitnessMethod* callee = as<IRLookupWitnessMethod>(callInst->getCallee());
        if (!callee)
            return false;

        // If the lookup depends on more than one existential base, we can't
        // proceed (it creates a cross-product of all the bases leading to exponential bloat).
        //
        auto existentialBaseResult = getUniqueExistentialBaseInst(parentFunc, callee);

        // Combine with the types of all the operands
        for (UInt i = 0; i < callInst->getArgCount(); i++)
        {
            auto arg = callInst->getArg(i);
            auto argBaseResult = getUniqueExistentialBaseInst(parentFunc, arg->getDataType());
            existentialBaseResult.combine(argBaseResult);
        }

        // Combine with existential bases from the call operand
        if (existentialBaseResult.flavor != ExistentialBaseResult::One)
            return false;

        // This is the existential base that we'll create new interface requirements for.
        auto baseInst = existentialBaseResult.baseInst;
        auto baseInterfaceType = as<IRInterfaceType>(baseInst->getDataType());

        // If we're dealing with an unspecialized base interface type, we can't proceed yet.
        if (!baseInterfaceType)
            return false;

        IRBuilder builder(module);
        builder.setInsertBefore(callInst);

        auto newCallee = translateMethodLookup(&builder, callee);
        if (newCallee == callee)
        {
            // If the new callee is the same as the old one, we don't need to do anything.
            return false;
        }

        // Use the callee's func-type to replace the arg types with the translated types.
        for (UInt i = 0; i < callInst->getArgCount(); i++)
        {
            auto arg = callInst->getArg(i);
            auto argType = arg->getDataType();
            auto newArgType = cast<IRFuncType>(newCallee->getDataType())->getParamType(i);
            if (argType != newArgType)
            {
                if (auto newArgOutType = as<IROutTypeBase>(newArgType))
                {
                    // Replace value type with new type (and de-duplicate)
                    builder.replaceOperand(
                        cast<IRPtrType>(argType)->getOperands() + 0,
                        newArgOutType->getValueType());
                }
                else
                    arg->setFullType(newArgType);
            }
        }

        // Replace the callInst's result type.
        auto resultType = callInst->getDataType();
        auto newResultType = cast<IRFuncType>(newCallee->getDataType())->getResultType();
        if (resultType != newResultType)
            callInst->setFullType(newResultType);

        // Replace the callee with the new callee.
        builder.replaceOperand(callInst->getCalleeUse(), newCallee);

        return true;
    }

    bool processFunc(IRFunc* func)
    {
        bool changed = false;
        for (auto bb : func->getBlocks())
        {
            for (auto inst : bb->getModifiableChildren())
            {
                if (auto callInst = as<IRCall>(inst))
                {
                    if (as<IRLookupWitnessMethod>(callInst->getCallee()))
                    {
                        changed |= processExistentialCall(callInst);
                    }
                }
            }
        }
        return changed;
    }

    bool lowerNewRequirements()
    {
        // Go through all interface extension definitions,
        // create a new interface type with new requirements.
        //
        // Then, we'll open up all the witness tables
        // for the given interface and insert the new entries
        // by performing the lookup defined by the chain.
        //

        IRBuilder builder(module);

        // First, we need to create the interface types for the associated type interfaces
        for (auto item : interfaceExtensionDefs)
        {
            IRInterfaceType* interfaceType = item.first;
            auto& def = item.second;

            // Create a new interface type with the requirements from the def
            List<IRInst*> allEntries;

            for (UIndex i = 0; i < interfaceType->getOperandCount(); i++)
            {
                allEntries.add(interfaceType->getOperand(i));
            }

            for (auto reqItem : def.newRequirements)
            {
                allEntries.add(
                    builder.createInterfaceRequirementEntry(reqItem.first, reqItem.second));
            }

            builder.setInsertBefore(module->getModuleInst());
            auto newInterfaceType =
                builder.createInterfaceType((UInt)allEntries.getCount(), allEntries.getBuffer());
            interfaceType->transferDecorationsTo(newInterfaceType);

            interfaceType->replaceUsesWith(newInterfaceType);

            // Update the base interface in the def
            def.base = newInterfaceType;

            // Gather all the witness tables that implement the base interface
            // and update them with the new requirements.
            //
            auto witnessTables = getWitnessTablesFromInterfaceType(module, def.base);

            for (auto witnessTable : witnessTables)
            {
                for (auto reqItem : def.newRequirements)
                {
                    auto key = reqItem.first;
                    auto chain = chains[key];

                    // Follow the chain to find the implementation in the witness table
                    IRInst* impl = witnessTable;
                    // Process the chain in reverse order
                    for (Int i = chain.keys.getCount() - 1; i >= 0; i--)
                    {
                        auto _key = chain.keys[i];
                        SLANG_ASSERT(as<IRWitnessTable>(impl));
                        impl = findWitnessTableEntry(as<IRWitnessTable>(impl), _key);
                    }

                    if (assocTypeInterfaces.containsKey(chain))
                    {
                        // Create a new witness table for (impl : assocTypeInterface)
                        auto assocTypeInterface = assocTypeInterfaces[chain].base;

                        // TODO: need to extend for generics?
                        auto newTable =
                            builder.createWitnessTable(assocTypeInterface, (IRType*)impl);
                    }

                    builder.createWitnessTableEntry(witnessTable, key, impl);
                }
            }
        }

        return interfaceExtensionDefs.getCount() > 0 || assocTypeInterfaces.getCount() > 0;
    }
};

bool lowerWitnessLookupChains(IRModule* module, DiagnosticSink* sink)
{
    bool changed = false;
    WitnessChainLookupLoweringContext context;
    context.module = module;
    context.sink = sink;
    context.init();

    for (auto inst : module->getGlobalInsts())
    {
        // Process all fully specialized functions and look for
        // witness lookup instructions. If we see a lookup for a non-static function,
        // create a dispatch function and replace the lookup with a call to the dispatch function.
        if (auto func = as<IRFunc>(inst))
            changed |= context.processFunc(func);
    }

    // Lower all collected requirements.
    changed |= context.lowerNewRequirements();

    return changed;
}
} // namespace Slang
