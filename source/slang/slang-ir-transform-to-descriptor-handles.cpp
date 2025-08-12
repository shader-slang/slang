// slang-ir-transform-to-descriptor-handles.cpp
//
// This file implements the IR transformation that converts resource types to descriptor handles.
// This is done by cloning the original stuct with resource fields replaced with descriptor handles.
//
// The transformation performs the following key operations:
// 1. Updates struct types that contain resources to use descriptor handles instead
// 2. Inserts cast instructions to convert between descriptor handles and resources

#include "slang-ir-transform-to-descriptor-handles.h"

#include "slang-ir-insts.h"
#include "slang-ir-inst-pass-base.h"
#include "slang-ir.h"
#include "slang-ir-util.h"
#include "slang-legalize-types.h"

namespace Slang
{
struct ResourceToDescriptorHandleTransformContext : public InstPassBase
{
    TargetProgram* targetProgram;
    DiagnosticSink* diagnosticSink;
    IRBuilder irBuilder;

    /// Maps original types to their lowered equivalents
    /// Key: Original type (e.g., struct with Texture2D field)
    /// Value: Lowered type (e.g., struct with DescriptorHandle<Texture2D> field)
    Dictionary<IRType*, IRType*> originalToLoweredTypeMap;

    /// Tracks struct fields that have been converted to descriptor handles
    /// These fields need special handling in access chains and function calls
    List<IRStructField*> fieldsConvertedToDescriptorHandles;

    /// Tracks parameter block types that have been updated with lowered element types
    List<IRParameterBlockType*> updatedParameterBlockTypes;

    ResourceToDescriptorHandleTransformContext(
        TargetProgram* inTargetProgram,
        IRModule* module,
        DiagnosticSink* inSink)
        : InstPassBase(module)
        , targetProgram(inTargetProgram)
        , diagnosticSink(inSink)
        , irBuilder(module)
    {
    }

    /// Recursively converts types to their descriptor handle equivalents
    ///
    /// This function performs the core type transformation:
    /// - Resource types (Texture2D, Buffer, etc.) -> DescriptorHandle<ResourceType>
    /// - Structs containing resources -> Structs with DescriptorHandle fields
    /// - Arrays/pointers of resources -> Arrays/pointers of DescriptorHandle types
    ///
    /// @param originalType The type to potentially transform
    /// @return The transformed type, or the original type if no transformation needed
    IRType* convertTypeToDescriptorHandle(IRType* originalType)
    {
        // Check if we've already processed this type to avoid infinite recursion
        if (auto existingLoweredType = originalToLoweredTypeMap.tryGetValue(originalType))
            return *existingLoweredType;

        IRType* transformedType = originalType;

        if (isResourceType(originalType))
        {
            // Transform: Texture2D -> DescriptorHandle<Texture2D>
            irBuilder.setInsertBefore(originalType);
            transformedType = (IRDescriptorHandleType*)irBuilder.getType(
                kIROp_DescriptorHandleType,
                originalType);
        }
        else if (auto structType = as<IRStructType>(originalType))
        {
            // Check if this struct contains any resource types that need transformation
            bool structNeedsTransformation = false;
            for (auto field : structType->getFields())
            {
                auto fieldType = field->getFieldType();
                if (convertTypeToDescriptorHandle(fieldType) != fieldType)
                {
                    structNeedsTransformation = true;
                    break;
                }
            }

            if (structNeedsTransformation)
            {
                // Create a new struct type with transformed field types
                irBuilder.setInsertBefore(structType);
                auto transformedStructType = irBuilder.createStructType();

                // Preserve decorations from the original struct
                copyNameHintAndDebugDecorations(transformedStructType, structType);

                // Transform all fields, converting resource types to descriptor handles
                for (auto originalField : structType->getFields())
                {
                    auto originalFieldType = originalField->getFieldType();
                    auto transformedFieldType = convertTypeToDescriptorHandle(originalFieldType);
                    auto fieldKey = originalField->getKey();

                    auto transformedField = irBuilder.createStructField(
                        transformedStructType,
                        fieldKey,
                        transformedFieldType);
                    fieldsConvertedToDescriptorHandles.add(transformedField);
                }

                transformedType = transformedStructType;
            }
        }
        else if (auto arrayType = as<IRArrayTypeBase>(originalType))
        {
            // Handle arrays: Array<Texture2D, N> -> Array<DescriptorHandle<Texture2D>, N>
            auto elementType = arrayType->getElementType();
            auto transformedElementType = convertTypeToDescriptorHandle(elementType);

            if (transformedElementType != elementType)
            {
                irBuilder.setInsertBefore(originalType);
                transformedType =
                    irBuilder.getArrayType(transformedElementType, arrayType->getElementCount());
            }
        }
        else if (auto ptrType = as<IRPtrTypeBase>(originalType))
        {
            // Handle pointers: Ptr<Texture2D> -> Ptr<DescriptorHandle<Texture2D>>
            auto valueType = ptrType->getValueType();
            auto transformedValueType = convertTypeToDescriptorHandle(valueType);

            if (transformedValueType != valueType)
            {
                irBuilder.setInsertBefore(originalType);
                transformedType =
                    irBuilder.getPtrTypeWithAddressSpace(transformedValueType, ptrType);
            }
        }

        // Cache the transformation result to avoid recomputing
        if (transformedType != originalType)
        {
            originalToLoweredTypeMap[originalType] = transformedType;
        }

        return transformedType;
    }

    /// Transforms parameter block types to use descriptor handles
    ///
    /// This method finds all ParameterBlock<T> types where T contains resources,
    /// and replaces them with ParameterBlock<T'> where T' uses descriptor handles.
    ///
    /// For example: ParameterBlock<MaterialData> where MaterialData contains Texture2D
    /// becomes: ParameterBlock<MaterialData_DescHandleTransformed> with DescriptorHandle<Texture2D>
    void transformParameterBlockTypes()
    {
        List<IRParameterBlockType*> parameterBlockTypesToTransform;

        // First pass: identify all parameter block types that need transformation
        for (auto globalInst : module->getGlobalInsts())
        {
            if (auto paramBlockType = as<IRParameterBlockType>(globalInst))
            {
                auto elementType = paramBlockType->getElementType();
                auto transformedElementType = convertTypeToDescriptorHandle(elementType);

                if (transformedElementType != elementType)
                {
                    parameterBlockTypesToTransform.add(paramBlockType);
                }
            }
        }

        // Second pass: replace each parameter block type with its transformed version
        for (auto originalParamBlockType : parameterBlockTypesToTransform)
        {
            auto originalElementType = originalParamBlockType->getElementType();
            auto transformedElementType = convertTypeToDescriptorHandle(originalElementType);

            // Create new parameter block type: ParameterBlock<TransformedType>
            auto transformedParamBlockType =
                irBuilder.getType(kIROp_ParameterBlockType, transformedElementType);

            // Replace all uses of the original type with the new type
            originalParamBlockType->replaceUsesWith(transformedParamBlockType);
            originalParamBlockType->removeAndDeallocate();

            // Track the transformed parameter block for later processing
            if (auto newParameterBlockType = as<IRParameterBlockType>(transformedParamBlockType))
            {
                updatedParameterBlockTypes.add(newParameterBlockType);
            }
        }
    }

    /// Extracts the underlying resource type from a descriptor handle type
    ///
    /// @param descriptorHandleType A DescriptorHandle<ResourceType> type
    /// @return The wrapped ResourceType, or nullptr if not a descriptor handle
    IRType* extractResourceTypeFromDescriptorHandle(IRType* descriptorHandleType)
    {
        if (auto descHandleType = as<IRDescriptorHandleType>(descriptorHandleType))
        {
            return descHandleType->getResourceType();
        }
        return nullptr;
    }

    /// Converts a transformed struct back to its original form for compatibility
    ///
    /// This method takes a struct with descriptor handle fields and reconstructs
    /// the original struct with resource fields by inserting cast instructions.
    /// Used when passing transformed structs to functions expecting original types.
    ///
    /// @param transformedStructValue The struct value with descriptor handle fields
    /// @param targetOriginalType The original struct type to convert to
    /// @return A new struct value with resource fields (via casts)
    IRInst* convertTransformedStructToOriginal(
        IRInst* transformedStructValue,
        IRType* targetOriginalType)
    {
        auto transformedStructType = as<IRStructType>(transformedStructValue->getFullType());
        auto targetStructType = as<IRStructType>(targetOriginalType);

        if (!transformedStructType || !targetStructType)
            return transformedStructValue;

        // Extract fields from transformed struct and convert them to original types
        List<IRInst*> convertedFieldValues;

        auto transformedFields = transformedStructType->getFields();
        auto targetFields = targetStructType->getFields();

        auto transformedFieldIter = transformedFields.begin();
        auto targetFieldIter = targetFields.begin();

        while (transformedFieldIter != transformedFields.end() &&
               targetFieldIter != targetFields.end())
        {
            auto transformedField = *transformedFieldIter;
            auto targetField = *targetFieldIter;

            // Extract the field value from the transformed struct
            auto fieldValue = irBuilder.emitFieldExtract(
                transformedField->getFieldType(),
                transformedStructValue,
                transformedField->getKey());

            // Convert descriptor handle fields back to resource types if needed
            auto convertedFieldValue =
                convertDescriptorHandleToOriginalType(fieldValue, targetField->getFieldType());
            convertedFieldValues.add(convertedFieldValue);

            ++transformedFieldIter;
            ++targetFieldIter;
        }

        // Reconstruct the original struct with converted field values
        return irBuilder.emitMakeStruct(targetOriginalType, convertedFieldValues);
    }

    /// Converts a value from transformed type back to its original type
    ///
    /// This method handles the reverse transformation, converting descriptor handles
    /// and transformed structs back to their original resource types and struct types.
    ///
    /// @param transformedValue The value with transformed type (descriptor handle or transformed
    /// struct)
    /// @param targetOriginalType The original type to convert to
    /// @return The converted value, or the original value if no conversion needed
    IRInst* convertDescriptorHandleToOriginalType(
        IRInst* transformedValue,
        IRType* targetOriginalType)
    {
        auto transformedValueType = transformedValue->getFullType();

        // Case 1: Convert descriptor handle back to resource type
        if (auto descriptorHandleType = as<IRDescriptorHandleType>(transformedValueType))
        {
            auto resourceType = extractResourceTypeFromDescriptorHandle(descriptorHandleType);
            if (resourceType && resourceType == targetOriginalType)
            {
                // Insert cast instruction: DescriptorHandle<Texture2D> -> Texture2D
                return irBuilder.emitIntrinsicInst(
                    resourceType,
                    kIROp_CastDescriptorHandleToResource,
                    1,
                    &transformedValue);
            }
        }
        // Case 2: Convert transformed struct back to original struct
        else if (as<IRStructType>(transformedValueType))
        {
            auto targetStructType = as<IRStructType>(targetOriginalType);
            if (targetStructType)
            {
                // Check if this transformed struct corresponds to the target original type
                IRType* expectedTransformedType = nullptr;
                if (originalToLoweredTypeMap.tryGetValue(
                        targetOriginalType,
                        expectedTransformedType) &&
                    expectedTransformedType == transformedValueType)
                {
                    return convertTransformedStructToOriginal(transformedValue, targetOriginalType);
                }
            }
        }

        // No conversion needed - return value as-is
        return transformedValue;
    }

    /// Categories of access chain origins for determining transformation behavior
    enum class AccessChainOrigin
    {
        FunctionParameter, /// Access originates from a function parameter (kIROp_Param)
        ParameterBlock,    /// Access originates from a global parameter block (IRGlobalParam)
        LocalVariable      /// Access originates from a local variable or other instruction
    };

    /// Information about an access chain's root and origin type
    struct AccessChainAnalysis
    {
        IRInst* rootInstruction;      /// The root instruction of the access chain
        AccessChainOrigin originType; /// Where the access chain originates from
    };

    /// Traces an instruction back through field accesses to find its ultimate origin
    ///
    /// This method follows a chain of field extracts, element accesses, loads, etc.
    /// back to the root instruction to understand where the access is coming from.
    /// This information is crucial for determining how to handle descriptor handle
    /// transformations properly.
    ///
    /// @param accessInstruction The instruction to trace back from
    /// @return Information about the access chain's origin
    AccessChainAnalysis traceAccessChainToOrigin(IRInst* accessInstruction)
    {
        // Follow the access chain backwards to find the root
        IRInst* currentInstruction = accessInstruction;

        while (currentInstruction)
        {
            switch (currentInstruction->getOp())
            {
            case kIROp_FieldExtract:
                {
                    // Follow field extraction: struct.field -> struct
                    auto fieldExtract = as<IRFieldExtract>(currentInstruction);
                    currentInstruction = fieldExtract->getBase();
                    break;
                }
                case kIROp_GetElement:
                {
                    // Follow array access: array[index] -> array
                    auto getElement = as<IRGetElement>(currentInstruction);
                    currentInstruction = getElement->getBase();
                    break;
                }
                case kIROp_FieldAddress:
                {
                    // Follow field address: &struct.field -> struct
                    auto fieldAddr = as<IRFieldAddress>(currentInstruction);
                    currentInstruction = fieldAddr->getBase();
                    break;
                }
                case kIROp_GetElementPtr:
                {
                    // Follow element pointer: &array[index] -> array
                    auto getElementPtr = as<IRGetElementPtr>(currentInstruction);
                    currentInstruction = getElementPtr->getBase();
                    break;
                }
                case kIROp_Load:
                {
                    // Follow load operation: load(ptr) -> ptr
                    auto load = as<IRLoad>(currentInstruction);
                    currentInstruction = load->getPtr();
                    break;
                }
                case kIROp_Param:
                {
                    // Reached a function parameter - this is a function parameter access
                    return {currentInstruction, AccessChainOrigin::FunctionParameter};
                }
                default:
                {
                    // Check if it's a global parameter (parameter block variable)
                    if (as<IRGlobalParam>(currentInstruction))
                    {
                        return {currentInstruction, AccessChainOrigin::ParameterBlock};
                    }

                    // Some other kind of instruction (local variable, etc.)
                    return {currentInstruction, AccessChainOrigin::LocalVariable};
                }
                }
            }

            // Couldn't trace to a recognizable origin
            return {nullptr, AccessChainOrigin::LocalVariable};
        }


        /// Processes an access chain to apply descriptor handle transformations
        ///
        /// This method recursively processes all users of an instruction to apply
        /// the necessary type transformations and insert cast instructions where needed.
        /// It handles field accesses, loads, stores, and function calls that need
        /// to work with the transformed types.
        ///
        /// @param accessInstruction The instruction whose users should be processed
        void processDescriptorHandleAccessChain(IRInst* accessInstruction)
        {
            // Process all users of this instruction
            for (auto use = accessInstruction->firstUse; use; use = use->nextUse)
            {
                auto userInstruction = use->getUser();

                switch (userInstruction->getOp())
                {
                case kIROp_FieldAddress:
                case kIROp_GetElementPtr:
                {
                    // Update pointer types for field/element addresses to point to transformed
                    // types
                    auto currentPtrType = as<IRPtrTypeBase>(userInstruction->getFullType());
                    auto originalValueType = currentPtrType->getValueType();
                    IRType* transformedValueType = nullptr;

                    if (originalToLoweredTypeMap.tryGetValue(
                            originalValueType,
                            transformedValueType))
                    {
                        // Create new pointer type pointing to the transformed type
                        auto transformedPtrType = irBuilder.getPtrTypeWithAddressSpace(
                            transformedValueType, // new value type (with descriptor handles)
                            currentPtrType);      // preserves pointer op and address space

                        // Update the instruction's type
                        userInstruction->setFullType(transformedPtrType);
                    }

                    // Continue processing the chain
                    processDescriptorHandleAccessChain(userInstruction);
                }
                break;

                case kIROp_RWStructuredBufferGetElementPtr:
                    // Pass through to next user (typically a Load operation)
                    processDescriptorHandleAccessChain(userInstruction);
                    break;

                case kIROp_GetElement:
                {
                    // Update the result type of array element access if needed
                    auto originalResultType = userInstruction->getFullType();
                    IRType* transformedResultType = nullptr;

                    if (originalToLoweredTypeMap.tryGetValue(
                            originalResultType,
                            transformedResultType))
                    {
                        userInstruction->setFullType(transformedResultType);
                        break;
                    }

                    processDescriptorHandleAccessChain(userInstruction);
                }
                break;
                case kIROp_FieldExtract:
                case kIROp_Load:
                {
                    // Analyze where this access originates from
                    AccessChainAnalysis chainAnalysis = traceAccessChainToOrigin(userInstruction);

                    // Only transform accesses from parameter blocks (not function params or locals)
                    if (chainAnalysis.originType != AccessChainOrigin::ParameterBlock)
                    {
                        break;
                    }

                    // Update the instruction's type to use descriptor handles if needed
                    auto originalResultType = userInstruction->getFullType();
                    IRType* transformedResultType = nullptr;

                    if (originalToLoweredTypeMap.tryGetValue(
                            originalResultType,
                            transformedResultType))
                    {
                        userInstruction->setFullType(transformedResultType);
                    }

                    // If the result is now a descriptor handle, insert a cast back to resource type
                    if (auto descriptorHandleType =
                            as<IRDescriptorHandleType>(transformedResultType))
                    {
                        auto resourceType =
                            extractResourceTypeFromDescriptorHandle(descriptorHandleType);
                        if (resourceType)
                        {
                            // Insert cast instruction after the load/extract:
                            // DescriptorHandle<Texture2D> -> Texture2D
                            irBuilder.setInsertAfter(userInstruction);
                            auto castInstruction = irBuilder.emitIntrinsicInst(
                                descriptorHandleType->getResourceType(),
                                kIROp_CastDescriptorHandleToResource,
                                1,
                                &userInstruction);

                            // Replace all uses of the original instruction with the cast result
                            // (except the use in the cast instruction itself)
                            List<IRUse*> usesToReplace;
                            for (auto originalUse = userInstruction->firstUse; originalUse;
                                 originalUse = originalUse->nextUse)
                            {
                                if (originalUse->getUser() != castInstruction)
                                {
                                    usesToReplace.add(originalUse);
                                }
                            }

                            for (auto useToReplace : usesToReplace)
                            {
                                useToReplace->set(castInstruction);
                            }
                        }
                    }

                    processDescriptorHandleAccessChain(userInstruction);
                }
                break;
                case kIROp_Store:
                {
                    // Handle storing transformed values into locations expecting original types
                    auto storeInstruction = userInstruction;
                    auto valueBeingStored = storeInstruction->getOperand(1); // The value to store
                    auto destinationPtr =
                        storeInstruction->getOperand(0); // The destination pointer

                    // Analyze the types involved
                    auto valueType = valueBeingStored->getFullType();
                    auto ptrType = as<IRPtrTypeBase>(destinationPtr->getFullType());
                    if (!ptrType) break;

                    auto expectedTargetType = ptrType->getValueType();

                    // Case 1: Storing a descriptor handle where a resource is expected
                    if (auto descriptorHandleType = as<IRDescriptorHandleType>(valueType))
                    {
                        auto resourceType =
                            extractResourceTypeFromDescriptorHandle(descriptorHandleType);
                        if (resourceType && resourceType == expectedTargetType)
                        {
                            // Cast descriptor handle back to resource type before storing
                            irBuilder.setInsertBefore(storeInstruction);
                            auto castInstruction = irBuilder.emitIntrinsicInst(
                                resourceType,
                                kIROp_CastDescriptorHandleToResource,
                                1,
                                &valueBeingStored);

                            // Update the store to use the cast result
                            storeInstruction->setOperand(1, castInstruction);
                        }
                    }
                    // Case 2: Storing a transformed struct where an original struct is expected
                    else if (as<IRStructType>(valueType))
                    {
                        auto expectedOriginalStructType = as<IRStructType>(expectedTargetType);
                        if (expectedOriginalStructType)
                        {
                            // Check if we're storing a transformed struct to an original struct
                            // location
                            IRType* expectedTransformedType = nullptr;
                            if (originalToLoweredTypeMap.tryGetValue(
                                    expectedTargetType,
                                    expectedTransformedType) &&
                                expectedTransformedType == valueType)
                            {
                                // Convert the transformed struct back to the original struct
                                irBuilder.setInsertBefore(storeInstruction);
                                auto convertedValue = convertTransformedStructToOriginal(
                                    valueBeingStored,
                                    expectedTargetType);
                                storeInstruction->setOperand(1, convertedValue);
                            }
                        }
                    }
                }
                break;
                case kIROp_Call:
                {
                    // Handle function calls where transformed arguments need conversion to original
                    // types
                    auto callInstruction = userInstruction;
                    auto calledFunction = callInstruction->getOperand(0);

                    // Analyze the function signature to check parameter types
                    if (auto functionType = as<IRFuncType>(calledFunction->getFullType()))
                    {
                        auto expectedParameterTypes = functionType->getParamTypes();
                        UInt parameterCount = (UInt)expectedParameterTypes.getCount();
                        UInt argumentCount =
                            callInstruction->getOperandCount() - 1; // Exclude function operand

                        // Process each argument against its corresponding parameter
                        for (UInt argIndex = 0;
                             argIndex < argumentCount && argIndex < parameterCount;
                             argIndex++)
                        {
                            auto argumentOperand =
                                callInstruction->getOperand(argIndex + 1); // +1 to skip function
                            auto actualArgumentType = argumentOperand->getFullType();
                            auto expectedParameterType = expectedParameterTypes[argIndex];

                            // Check if we're passing a transformed type where original type is
                            // expected
                            IRType* expectedTransformedType = nullptr;
                            if (originalToLoweredTypeMap.tryGetValue(
                                    expectedParameterType,
                                    expectedTransformedType) &&
                                expectedTransformedType == actualArgumentType)
                            {
                                // Convert the transformed argument back to the original type
                                irBuilder.setInsertBefore(callInstruction);
                                auto convertedArgument = convertDescriptorHandleToOriginalType(
                                    argumentOperand,
                                    expectedParameterType);
                                callInstruction->setOperand(argIndex + 1, convertedArgument);
                            }
                        }
                    }
                }
                break;

                default:
                    // Other instruction types don't require special handling
                    break;
                }
            }
        }

        /// Applies descriptor handle transformations to the entire module
        ///
        /// This method coordinates the transformation process by:
        /// 1. Processing converted struct fields to handle their access chains
        /// 2. Processing parameter block accesses that use the transformed types
        void applyDescriptorHandleTransformations()
        {
            // Process all struct fields that were converted to descriptor handles
            for (auto convertedField : fieldsConvertedToDescriptorHandles)
            {
                // Process the access chain for each converted field
                // This handles loads, stores, and other operations on these fields
                auto fieldKey = convertedField->getKey();
                processDescriptorHandleAccessChain(fieldKey);
            }

            // Process parameter block accesses that use the transformed types
            // Examples:
            // - ParameterBlock<Array<ResourceStruct, 4>> arrayBlock; arrayBlock[1];
            // - ParameterBlock<NestedStruct> nestedBlock; NestedStruct nested = nestedBlock;
            for (auto transformedParameterBlockType : updatedParameterBlockTypes)
            {
                for (auto use = transformedParameterBlockType->firstUse; use; use = use->nextUse)
                {
                    auto userInstruction = use->getUser();
                    if (auto globalParameterVariable = as<IRGlobalParam>(userInstruction))
                    {
                        processDescriptorHandleAccessChain(globalParameterVariable);
                    }
                }
            }
        }

        /// Main entry point for processing the entire module
        ///
        /// Orchestrates the complete descriptor handle transformation:
        /// 1. Transforms parameter block types to use descriptor handles
        /// 2. Applies transformations to all access chains and instructions
        void processModule()
        {
            transformParameterBlockTypes();
            applyDescriptorHandleTransformations();
        }
    };

    /// Public API for transforming resource types to descriptor handles
    ///
    /// This function transforms an IR module to use explicit descriptor handles
    ///
    /// @param targetProgram The target program being compiled
    /// @param module The IR module to transform
    /// @param diagnosticSink For reporting any errors or warnings during transformation
    void transformResourceTypesToDescriptorHandles(
        TargetProgram* targetProgram,
        IRModule* module,
        DiagnosticSink* diagnosticSink)
    {
        ResourceToDescriptorHandleTransformContext transformContext(
            targetProgram,
            module,
            diagnosticSink);
        transformContext.processModule();
    }
}
