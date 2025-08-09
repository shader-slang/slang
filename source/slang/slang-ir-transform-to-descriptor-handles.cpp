// slang-ir-transform-to-descriptor-handles.cpp
#include "slang-ir-transform-to-descriptor-handles.h"

#include "slang-ir-insts.h"
#include "slang-ir-inst-pass-base.h"
#include "slang-ir.h"
#include "slang-ir-util.h"
#include "slang-legalize-types.h"

namespace Slang
{
struct ResourceToDescriptorHandleContext : public InstPassBase
{
    TargetProgram* targetProgram;
    DiagnosticSink* sink;
    IRBuilder builder;

    // Dictionary mapping original types to lowered types
    // Original resource types -> lowered resource types
    Dictionary<IRType*, IRType*> typeLoweringMap;

    // Track which fields have been updated to descriptor handles
    // for updating the instructions that access these fields
    List<IRStructField*> updatedFields;
    List<IRParameterBlockType*> updatedParameterBlock;


    ResourceToDescriptorHandleContext(
        TargetProgram* inTargetProgram,
        IRModule* module,
        DiagnosticSink* inSink)
        : InstPassBase(module), targetProgram(inTargetProgram), sink(inSink), builder(module)
    {
    }

    // Recursive type lowering function
    IRType* convertTypeToHandler(IRType* originalType)
    {
        // Check if we've already lowered this type
        if (auto existing = typeLoweringMap.tryGetValue(originalType))
            return *existing;

        IRType* loweredType = originalType;

        if (isResourceType(originalType))
        {
            // Lower resource types to DescriptorHandle<ResourceType>
            builder.setInsertBefore(originalType);
            loweredType =
                (IRDescriptorHandleType*)builder.getType(kIROp_DescriptorHandleType, originalType);
        }
        else if (auto structType = as<IRStructType>(originalType))
        {
            // Check if this struct contains any resource types that need lowering
            bool needsLowering = false;
            for (auto field : structType->getFields())
            {
                auto fieldType = field->getFieldType();
                if (convertTypeToHandler(fieldType) != fieldType)
                {
                    needsLowering = true;
                    break;
                }
            }

            if (needsLowering)
            {
                // Clone the struct with lowered field types
                builder.setInsertBefore(structType);
                auto newStructType = builder.createStructType();

                // Copy decorations from original
                copyNameHintAndDebugDecorations(newStructType, structType);

                // Clone all fields with potentially lowered types
                for (auto originalField : structType->getFields())
                {
                    auto fieldType = originalField->getFieldType();
                    auto loweredFieldType = convertTypeToHandler(fieldType);
                    auto fieldKey = originalField->getKey();

                    auto newField =
                        builder.createStructField(newStructType, fieldKey, loweredFieldType);

                    updatedFields.add(newField);
                }

                loweredType = newStructType;
            }
        }
        else if (auto arrayType = as<IRArrayTypeBase>(originalType))
        {
            auto elementType = arrayType->getElementType();
            auto loweredElementType = convertTypeToHandler(elementType);

            if (loweredElementType != elementType)
            {
                builder.setInsertBefore(originalType);
                loweredType =
                    builder.getArrayType(loweredElementType, arrayType->getElementCount());
            }
        }
        else if (auto ptrType = as<IRPtrTypeBase>(originalType))
        {
            auto valueType = ptrType->getValueType();
            auto loweredValueType = convertTypeToHandler(valueType);

            if (loweredValueType != valueType)
            {
                builder.setInsertBefore(originalType);
                loweredType = builder.getPtrTypeWithAddressSpace(loweredValueType, ptrType);
            }
        }

        if (loweredType != originalType)
        {
            typeLoweringMap[originalType] = loweredType;
        }

        return loweredType;
    }


    // Pass 1: Clone parameter block structs with lowered types
    void cloneParameterBlockStructs()
    {
        List<IRParameterBlockType*> paramBlockTypesToReplace;

        // First, collect all parameter block types that need replacement
        for (auto globalInst : module->getGlobalInsts())
        {
            if (auto paramBlockType = as<IRParameterBlockType>(globalInst))
            {
                auto elementType = paramBlockType->getElementType();
                auto loweredElementType = convertTypeToHandler(elementType);

                if (loweredElementType != elementType)
                {
                    paramBlockTypesToReplace.add(paramBlockType);
                }
            }
        }

        // Then replace each parameter block type with a new one
        for (auto paramBlockType : paramBlockTypesToReplace)
        {
            auto elementType = paramBlockType->getElementType();
            auto loweredElementType = convertTypeToHandler(elementType);


            // Create a new parameter block type with the lowered element type
            auto newParamBlockType = builder.getType(kIROp_ParameterBlockType, loweredElementType);

            paramBlockType->replaceUsesWith(newParamBlockType);
            paramBlockType->removeAndDeallocate();

            if (auto arrayType = as<IRArrayType>(loweredElementType))
            {
                if (auto parameterBlockType = as<IRParameterBlockType>(newParamBlockType))
                {
                    updatedParameterBlock.add(parameterBlockType);
                }
            }
        }
    }

    // Helper to extract the resource type from a DescriptorHandle<ResourceType>
    IRType* getResourceTypeFromDescriptorHandle(IRType* descriptorHandleType)
    {
        if (auto descHandleType = as<IRDescriptorHandleType>(descriptorHandleType))
        {
            return descHandleType->getResourceType();
        }
        return nullptr;
    }

    // Recursively follow through address instructions until we find a load or other use
    void processAccessChain(IRInst* accessInst)
    {
        // Process certain users of this address instruction
        for (auto use = accessInst->firstUse; use; use = use->nextUse)
        {
            auto user = use->getUser();

            switch (user->getOp())
            {
            case kIROp_FieldAddress:
            case kIROp_GetElementPtr:
                {
                    processAccessChain(user);

                    // If this is a field address, we need to the ptrType
                    auto currentPtrType = as<IRPtrTypeBase>(user->getFullType());
                    auto originalType = currentPtrType->getValueType();
                    IRType* typeLowered = nullptr;
                    if (typeLoweringMap.tryGetValue(originalType, typeLowered))
                    {
                        // Create new pointer type pointing to the lowered type
                        auto ptrToTypeLowered = builder.getPtrTypeWithAddressSpace(
                            typeLowered,     // new value type
                            currentPtrType); // preserves op + address space

                        // Update the field address type
                        user->setFullType(ptrToTypeLowered);
                    }
                }
                break;
            case kIROp_RWStructuredBufferGetElementPtr:
                processAccessChain(user); // pass through to next user (e.g. Load)
                break;
            case kIROp_GetElement:
                {
                    processAccessChain(user);
                    auto originalType = user->getFullType();
                    IRType* typeLowered = nullptr;
                    if (typeLoweringMap.tryGetValue(originalType, typeLowered))
                    {
                        user->setFullType(typeLowered);
                        break;
                    }
                }
                break;
            case kIROp_FieldExtract:
            case kIROp_Load:
                {
                    processAccessChain(user);

                    // Load now should load a descriptor handle ptr type
                    auto originalType = user->getFullType();
                    IRType* typeLowered = nullptr;
                    if (typeLoweringMap.tryGetValue(originalType, typeLowered))
                    {
                        user->setFullType(typeLowered);
                    }

                    if (auto descriptorHandleType = as<IRDescriptorHandleType>(typeLowered))
                    {
                        auto resourceType =
                            getResourceTypeFromDescriptorHandle(descriptorHandleType);
                        if (resourceType)
                        {
                            // Insert CastDescriptorHandleToResource after the load
                            builder.setInsertAfter(user);
                            // castInst: cast the descriptor handle back to resource type
                            auto castInst = builder.emitIntrinsicInst(
                                descriptorHandleType->getResourceType(),
                                kIROp_CastDescriptorHandleToResource,
                                1,
                                &user);


                            // Replace all uses of the load with the cast result
                            // But we need to be careful not to replace the use in the cast itself
                            List<IRUse*> usesToReplace;
                            for (auto loadUse = user->firstUse; loadUse; loadUse = loadUse->nextUse)
                            {
                                if (loadUse->getUser() != castInst)
                                {
                                    usesToReplace.add(loadUse);
                                }
                            }

                            for (auto useToReplace : usesToReplace)
                            {
                                useToReplace->set(castInst);
                            }
                        }
                    }
                }
                break;

            default:
                // For other uses, we don't need to do anything special
                break;
            }
            }
        }

        // Pass 2: Process parameter block users and insert conversion logic
        void updateTypeAndInsertCastLogic()
        {

            for (auto field : updatedFields)
            {
                // For each updated field, we need to process its address chain
                // This will handle cases where the field is used in loads or other instructions
                auto structKey = field->getKey();
                if (!structKey)
                {
                    // If the field has no key, we can't process it
                    continue;
                }

                processAccessChain(structKey);
            }

            // Handle array inside ParameterBlock and gets accessed with GetElementPtr directly
            // e.g. ParameterBlock<Array<ResourceStruct, 4>> arrayBlock; arrayBlock[1];
            for (auto arrayType : updatedParameterBlock)
            {
                for (auto use = arrayType->firstUse; use; use = use->nextUse)
                {
                    auto user = use->getUser();
                    if (auto globalParam = as<IRGlobalParam>(user))
                    {
                        processAccessChain(globalParam);
                    }
                }
            }
        }

        void processModule()
        {
            cloneParameterBlockStructs();
            updateTypeAndInsertCastLogic();
        }
};

void transformResourceTypesToDescriptorHandles(
    TargetProgram* targetProgram,
    IRModule* module,
    DiagnosticSink* sink)
{
    ResourceToDescriptorHandleContext context(targetProgram, module, sink);
    context.processModule();
}
}
