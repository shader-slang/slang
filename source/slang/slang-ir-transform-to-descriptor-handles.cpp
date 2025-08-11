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
        // Lowered types are either DescriptorHandle<ResourceType> or a struct with DescriptorHandle<ResourceType> 
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

        // Recursively convert types, lowering resource types to descriptor handles,
        // and lowering any structs that contain resource types to a struct with
        // DescriptorHandle<ResourceType> recursively.
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
                loweredType = (IRDescriptorHandleType*)builder.getType(kIROp_DescriptorHandleType, originalType);
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
                    // Create a new struct type
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

                        auto newField = builder.createStructField(newStructType, fieldKey, loweredFieldType);
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
                    loweredType = builder.getArrayType(loweredElementType, arrayType->getElementCount());
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

        // Clone parameter block structs with lowered types
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

                if (auto parameterBlockType = as<IRParameterBlockType>(newParamBlockType))
                {
                    updatedParameterBlock.add(parameterBlockType);
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

        // Convert a lowered struct (with descriptor handles) back to original struct (with resources)
        IRInst* convertLoweredStructToOriginal(IRInst* loweredValue, IRType* targetOriginalType)
        {
            auto loweredStructType = as<IRStructType>(loweredValue->getFullType());
            auto targetStructType = as<IRStructType>(targetOriginalType);

            if (!loweredStructType || !targetStructType)
                return loweredValue;

            // Create a new struct value by extracting fields from lowered struct and converting them
            List<IRInst*> fieldValues;

            auto loweredFields = loweredStructType->getFields();
            auto targetFields = targetStructType->getFields();

            auto loweredFieldIter = loweredFields.begin();
            auto targetFieldIter = targetFields.begin();

            while (loweredFieldIter != loweredFields.end() && targetFieldIter != targetFields.end())
            {
                auto loweredField = *loweredFieldIter;
                auto targetField = *targetFieldIter;

                // Extract the field value from the lowered struct
                auto fieldValue = builder.emitFieldExtract(
                    loweredField->getFieldType(),
                    loweredValue,
                    loweredField->getKey());

                // Convert the field value if needed
                auto convertedFieldValue = convertValueToOriginalType(fieldValue, targetField->getFieldType());
                fieldValues.add(convertedFieldValue);

                ++loweredFieldIter;
                ++targetFieldIter;
            }

            // Create the original struct with converted field values
            return builder.emitMakeStruct(targetOriginalType, fieldValues);
        }

        // Convert a value from lowered type to original type
        IRInst* convertValueToOriginalType(IRInst* value, IRType* targetType)
        {
            auto valueType = value->getFullType();

            // If it's a descriptor handle that needs to be converted to resource
            if (auto descriptorHandleType = as<IRDescriptorHandleType>(valueType))
            {
                auto resourceType = getResourceTypeFromDescriptorHandle(descriptorHandleType);
                if (resourceType && resourceType == targetType)
                {
                    return builder.emitIntrinsicInst(
                        resourceType,
                        kIROp_CastDescriptorHandleToResource,
                        1,
                        &value);
                }
            }
            // If it's a lowered struct that needs to be converted to original struct
            else if (auto loweredStructType = as<IRStructType>(valueType))
            {
                auto targetStructType = as<IRStructType>(targetType);
                if (targetStructType)
                {
                    IRType* expectedLoweredType = nullptr;
                    if (typeLoweringMap.tryGetValue(targetType, expectedLoweredType) &&
                        expectedLoweredType == valueType)
                    {
                        return convertLoweredStructToOriginal(value, targetType);
                    }
                }
            }

            // No conversion needed
            return value;
        }

        // Recursively process address instructions and their users
        void processAccessChain(IRInst* accessInst)
        {
            // Process users of this address instruction
            for (auto use = accessInst->firstUse; use; use = use->nextUse)
            {
                auto user = use->getUser();

                switch (user->getOp())
                {
                case kIROp_FieldAddress:
                case kIROp_GetElementPtr:
                {
                    // Update pointer type to point to lowered type if needed
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
                    processAccessChain(user);
                }
                break;
                case kIROp_RWStructuredBufferGetElementPtr:
                    processAccessChain(user); // Pass through to next user (e.g. Load)
                    break;
                case kIROp_GetElement:
                {
                    auto originalType = user->getFullType();
                    IRType* typeLowered = nullptr;
                    if (typeLoweringMap.tryGetValue(originalType, typeLowered))
                    {
                        user->setFullType(typeLowered);
                        break;
                    }
                    processAccessChain(user);
                }
                break;
                case kIROp_FieldExtract:
                case kIROp_Load:
                {

                    // Update type to descriptor handle if needed
                    auto originalType = user->getFullType();
                    IRType* typeLowered = nullptr;
                    if (typeLoweringMap.tryGetValue(originalType, typeLowered))
                    {
                        user->setFullType(typeLowered);
                    }

                    if (auto descriptorHandleType = as<IRDescriptorHandleType>(typeLowered))
                    {
                        auto resourceType = getResourceTypeFromDescriptorHandle(descriptorHandleType);
                        if (resourceType)
                        {
                            // Insert CastDescriptorHandleToResource after the load
                            builder.setInsertAfter(user);
                            auto castInst = builder.emitIntrinsicInst(
                                descriptorHandleType->getResourceType(),
                                kIROp_CastDescriptorHandleToResource,
                                1,
                                &user);

                            // Replace all uses of the load with the cast result
                            // (except the use in the cast instruction itself)
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

                    processAccessChain(user);
                }
                break;
                case kIROp_Store:
                {
                    // Handle the case where we're storing a lowered type into an original type location
                    auto storeInst = user;
                    auto valueOperand = storeInst->getOperand(1); // The value being stored
                    auto ptrOperand = storeInst->getOperand(0);   // The destination pointer

                    // Get the types
                    auto valueType = valueOperand->getFullType();
                    auto ptrType = as<IRPtrTypeBase>(ptrOperand->getFullType());
                    if (!ptrType) break;

                    auto targetType = ptrType->getValueType();

                    // Case 1: Storing a descriptor handle into a resource location
                    if (auto descriptorHandleType = as<IRDescriptorHandleType>(valueType))
                    {
                        auto resourceType = getResourceTypeFromDescriptorHandle(descriptorHandleType);
                        if (resourceType && resourceType == targetType)
                        {
                            // We need to cast the descriptor handle back to the resource type
                            builder.setInsertBefore(storeInst);
                            auto castInst = builder.emitIntrinsicInst(
                                resourceType,
                                kIROp_CastDescriptorHandleToResource,
                                1,
                                &valueOperand);

                            // Replace the value operand in the store with the cast result
                            storeInst->setOperand(1, castInst);
                        }
                    }
                    // Case 2: Storing a lowered struct into an original struct location
                    else if (auto sourceLoweredStruct = as<IRStructType>(valueType))
                    {
                        auto targetOriginalStruct = as<IRStructType>(targetType);
                        if (targetOriginalStruct)
                        {
                            // Check if this is a lowered struct being stored to original struct
                            // Look for the reverse mapping: find if targetType maps to sourceType
                            IRType* expectedLoweredType = nullptr;
                            if (typeLoweringMap.tryGetValue(targetType, expectedLoweredType) &&
                                expectedLoweredType == valueType)
                            {
                                // We need to convert the lowered struct back to the original struct
                                builder.setInsertBefore(storeInst);
                                auto convertedValue = convertLoweredStructToOriginal(valueOperand, targetType);
                                storeInst->setOperand(1, convertedValue);
                            }
                        }
                    }
                }
                break;
                case kIROp_Call:
                {
                    // Handle function calls where lowered type arguments need to be converted to original types
                    auto callInst = user;
                    auto funcOperand = callInst->getOperand(0);

                    // Get the function type to check parameter types
                    if (auto funcType = as<IRFuncType>(funcOperand->getFullType()))
                    {
                        auto paramTypes = funcType->getParamTypes();
                        auto argCount = callInst->getOperandCount() - 1; // Subtract 1 for function operand

                        // Check each argument against its corresponding parameter type
                        for (auto i = 0; i < argCount && i < paramTypes.getCount(); i++)
                        {
                            auto argOperand = callInst->getOperand(i + 1); // +1 to skip function operand
                            auto argType = argOperand->getFullType();
                            auto expectedParamType = paramTypes[i];

                            // Check if we're passing a lowered type to a function expecting original type
                            IRType* expectedLoweredType = nullptr;
                            if (typeLoweringMap.tryGetValue(expectedParamType, expectedLoweredType) &&
                                expectedLoweredType == argType)
                            {
                                // Convert the lowered argument back to original type
                                builder.setInsertBefore(callInst);
                                auto convertedArg = convertValueToOriginalType(argOperand, expectedParamType);
                                callInst->setOperand(i + 1, convertedArg);
                            }
                        }
                    }
                }
                break;

                default:
                    // For other uses, no special handling needed
                    break;
                }
            }
        }

        // Update types and insert cast logic for descriptor handle conversions
        void updateTypeAndInsertCastLogic()
        {
            for (auto field : updatedFields)
            {
                // Process the address chain for each updated field
                // This handles cases where the field is used in loads or other instructions
                auto structKey = field->getKey();
                if (!structKey)
                {
                    // Skip fields without keys
                    continue;
                }

                processAccessChain(structKey);
            }

            // Handle the cases where the updated parameter block is accessed directly.
            // e.g. ParameterBlock<Array<ResourceStruct, 4>> arrayBlock; arrayBlock[1];
            // or ParameterBlock<NestedStruct> nestedBlock; NestedStruct nested = nestedBlock;
            for (auto updatedType : updatedParameterBlock)
            {
                for (auto use = updatedType->firstUse; use; use = use->nextUse)
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
