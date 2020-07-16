//slang-ir-generics-lowering-context.cpp

#include "slang-ir-generics-lowering-context.h"

#include "slang-ir-layout.h"

namespace Slang
{
    bool isPolymorphicType(IRInst* typeInst)
    {
        if (as<IRParam>(typeInst) && as<IRTypeType>(typeInst->getDataType()))
            return true;
        switch (typeInst->op)
        {
        case kIROp_ThisType:
        case kIROp_AssociatedType:
        case kIROp_InterfaceType:
        case kIROp_lookup_interface_method:
            return true;
        case kIROp_Specialize:
        {
            for (UInt i = 0; i < typeInst->getOperandCount(); i++)
            {
                if (isPolymorphicType(typeInst->getOperand(i)))
                    return true;
            }
            return false;
        }
        default:
            break;
        }
        if (auto ptrType = as<IRPtrTypeBase>(typeInst))
        {
            return isPolymorphicType(ptrType->getValueType());
        }
        return false;
    }

    bool isTypeValue(IRInst* typeInst)
    {
        if (typeInst)
        {
            switch (typeInst->op)
            {
            case kIROp_TypeType:
                return true;
            case kIROp_lookup_interface_method:
                return typeInst->getDataType()->op == kIROp_TypeKind;
            default:
                return false;
            }
        }
        return false;
    }

    IRInst* SharedGenericsLoweringContext::maybeEmitRTTIObject(IRInst* typeInst)
    {
        IRInst* result = nullptr;
        if (mapTypeToRTTIObject.TryGetValue(typeInst, result))
            return result;
        IRBuilder builderStorage;
        auto builder = &builderStorage;
        builder->sharedBuilder = &sharedBuilderStorage;
        builder->setInsertBefore(typeInst->next);

        result = builder->emitMakeRTTIObject(typeInst);

        // For now the only type info we encapsualte is type size.
        IRSizeAndAlignment sizeAndAlignment;
        getNaturalSizeAndAlignment((IRType*)typeInst, &sizeAndAlignment);
        builder->addRTTITypeSizeDecoration(result, sizeAndAlignment.size);

        // Give a name to the rtti object.
        if (auto exportDecoration = typeInst->findDecoration<IRExportDecoration>())
        {
            String rttiObjName = String(exportDecoration->getMangledName()) + "_rtti";
            builder->addExportDecoration(result, rttiObjName.getUnownedSlice());
        }
        mapTypeToRTTIObject[typeInst] = result;
        return result;
    }

    IRInst* SharedGenericsLoweringContext::findInterfaceRequirementVal(IRInterfaceType* interfaceType, IRInst* requirementKey)
    {
        if (auto dict = mapInterfaceRequirementKeyValue.TryGetValue(interfaceType))
            return (*dict)[requirementKey].GetValue();
        _builldInterfaceRequirementMap(interfaceType);
        return findInterfaceRequirementVal(interfaceType, requirementKey);
    }

    void SharedGenericsLoweringContext::_builldInterfaceRequirementMap(IRInterfaceType* interfaceType)
    {
        mapInterfaceRequirementKeyValue.Add(interfaceType,
            Dictionary<IRInst*, IRInst*>());
        auto dict = mapInterfaceRequirementKeyValue.TryGetValue(interfaceType);
        for (UInt i = 0; i < interfaceType->getOperandCount(); i++)
        {
            auto entry = cast<IRInterfaceRequirementEntry>(interfaceType->getOperand(i));
            (*dict)[entry->getRequirementKey()] = entry->getRequirementVal();
        }
    }
}
