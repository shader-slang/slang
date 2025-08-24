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
    Dictionary<IRType*, IRType*> loweredToOriginalTypeMap;

    /// Tracks parameter block types that have been updated with lowered element types
    List<IRParameterBlockType*> updatedParameterBlockTypes;

    /// Maps original buffer types to their wrapper struct types
    Dictionary<IRType*, IRStructType*> bufferToWrapperMap;

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
                    irBuilder.createStructField(
                        transformedStructType,
                        fieldKey,
                        transformedFieldType);
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
            loweredToOriginalTypeMap[transformedType] = originalType;
        }

        return transformedType;
    }

    /// Checks if a type is a buffer type that results in a pointer in Metal
    ///
    /// These buffer types cannot be used directly in ParameterBlocks in Metal
    /// because they result in pointer types that create invalid syntax like
    /// "float device* constant*". These need to be wrapped in structs.
    ///
    /// @param type The type to check
    /// @return true if the type needs to be wrapped in a struct
    bool isBufferTypeThatNeedsWrapping(IRType* type)
    {
        switch (type->getOp())
        {
        // Structured buffer types - these emit as "ElementType device*" in Metal
        case kIROp_HLSLStructuredBufferType:
        case kIROp_HLSLRWStructuredBufferType:
        case kIROp_HLSLAppendStructuredBufferType:
        case kIROp_HLSLConsumeStructuredBufferType:
        case kIROp_HLSLRasterizerOrderedStructuredBufferType:
        
        // Byte address buffer types - these emit as "uint32_t device*" in Metal
        case kIROp_HLSLByteAddressBufferType:
        case kIROp_HLSLRWByteAddressBufferType:
        case kIROp_HLSLRasterizerOrderedByteAddressBufferType:
            return true;
            
        default:
            return false;
        }
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
        List<IRParameterBlockType*> bufferParameterBlocks;

        // First pass: identify all parameter block types that need transformation
        for (auto globalInst : module->getGlobalInsts())
        {
            if (auto paramBlockType = as<IRParameterBlockType>(globalInst))
            {
                auto elementType = paramBlockType->getElementType();
                
                // Check if this is ParameterBlock<BufferType> that needs wrapping
                if (isBufferTypeThatNeedsWrapping(elementType))
                {
                    // We will handle buffer types separately to wrap them in structs
                    bufferParameterBlocks.add(paramBlockType);
                }
            }
        }
        // Handle ParameterBlock<BufferType> wrapping
        handleBufferParameterBlockWrapping(bufferParameterBlocks);

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

    /// Handles wrapping of ParameterBlock<BufferType> into ParameterBlock<WrapperStruct>
    ///
    /// This method processes buffer parameter blocks that need wrapping in Metal.
    /// For each ParameterBlock<BufferType>, it:
    /// 1. Creates a wrapper struct with a single field containing the buffer
    /// 2. Transforms the type to ParameterBlock<WrapperStruct>
    /// 3. Updates all parameter uses to extract the buffer from the wrapper
    ///
    /// @param bufferParameterBlocks List of parameter block types that contain buffers
    void handleBufferParameterBlockWrapping(const List<IRParameterBlockType*>& bufferParameterBlocks)
    {
        for (auto originalParamBlockType : bufferParameterBlocks)
        {
            auto bufferType = originalParamBlockType->getElementType();

            // Check if we already created a wrapper for this buffer type
            IRStructType* wrapperStructType = nullptr;
            if (!bufferToWrapperMap.tryGetValue(bufferType, wrapperStructType))
            {
                // Create a new wrapper struct with a single field
                irBuilder.setInsertBefore(originalParamBlockType);
                wrapperStructType = irBuilder.createStructType();

                // Generate a descriptive name for the wrapper struct
                String wrapperName = "BufferWrapper";
                irBuilder.addNameHintDecoration(wrapperStructType, wrapperName.getUnownedSlice());

                // Create the single field containing the buffer
                auto fieldKey = irBuilder.createStructKey();
                
                String fieldName = "buffer"; // Default name
                irBuilder.addNameHintDecoration(fieldKey, fieldName.getUnownedSlice());
                irBuilder.createStructField(
                    wrapperStructType,
                    fieldKey,
                    bufferType);

                // Cache the wrapper struct for this buffer type
                bufferToWrapperMap[bufferType] = wrapperStructType;
            }

            // Create new parameter block type: ParameterBlock<WrapperStruct>
            auto transformedParamBlockType =
                irBuilder.getType(kIROp_ParameterBlockType, wrapperStructType);

            // Collect all uses before replacement to process them
            List<IRUse*> usesToUpdate;
            for (auto use = originalParamBlockType->firstUse; use; use = use->nextUse)
            {
                usesToUpdate.add(use);
            }

            // Replace the parameter block type
            originalParamBlockType->replaceUsesWith(transformedParamBlockType);

            // Now process all the uses to insert field extraction where needed
            for (auto use : usesToUpdate)
            {
                auto userInst = use->getUser();

                // Skip if this is just a type reference (not an actual parameter/variable)
                if (userInst->getOp() != kIROp_GlobalParam && userInst->getOp() != kIROp_Param)
                    continue;

                // Process all uses of this parameter/variable to insert field extraction
                List<IRUse*> parameterUses;
                for (auto paramUse = userInst->firstUse; paramUse; paramUse = paramUse->nextUse)
                {
                    parameterUses.add(paramUse);
                }

                for (auto paramUse : parameterUses)
                {
                    auto paramUserInst = paramUse->getUser();
                    // Insert before the user instruction
                    irBuilder.setInsertBefore(paramUserInst);
                    // Load the wrapper struct
                    auto wrapperValue = irBuilder.emitLoad(userInst);
                    // Extract the buffer field
                    auto firstField = wrapperStructType->getFields().getFirst();
                    auto bufferValue = irBuilder.emitFieldExtract(
                        firstField->getFieldType(),
                        wrapperValue,
                        firstField->getKey());
                    // Create storage for the buffer
                    auto bufferPtr = irBuilder.emitVar(firstField->getFieldType());
                    irBuilder.emitStore(bufferPtr, bufferValue);
                    // Replace the use of the wrapper parameter with the extracted buffer
                    paramUse->set(bufferPtr);
                }
            }

            originalParamBlockType->removeAndDeallocate();
        }
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
        // Case 3: Convert PtrType<transformed struct> back to PtrType<original struct>
        else if (auto transformedPtrType = as<IRPtrType>(transformedValueType))
        {
            auto targetPtrType = as<IRPtrType>(targetOriginalType);
            if (targetPtrType)
            {
                auto transformedPointeeType = transformedPtrType->getValueType();
                auto targetPointeeType = targetPtrType->getValueType();
                
                // Check if the pointer's value type corresponds to a transformed type
                IRType* expectedTransformedPointeeType = nullptr;
                if (originalToLoweredTypeMap.tryGetValue(
                        targetPointeeType,
                        expectedTransformedPointeeType) &&
                    expectedTransformedPointeeType == transformedPointeeType)
                {
                    // We need to convert the value that the pointer points to, not just the pointer type
                    // 1. Load the transformed struct from the pointer
                    auto transformedStructValue = irBuilder.emitLoad(transformedValue);
                    
                    // 2. Convert the loaded transformed struct to the original struct
                    auto convertedStructValue = convertTransformedStructToOriginal(
                        transformedStructValue, 
                        targetPointeeType);
                    
                    // 3. Allocate storage for the converted struct and store it
                    auto convertedStructPtr = irBuilder.emitVar(targetPointeeType);
                    irBuilder.emitStore(convertedStructPtr, convertedStructValue);
                    
                    return convertedStructPtr;
                }
            }
        }
        // Case 4: Convert Array<transformed_type> back to Array<original_type>
        else if (auto transformedArrayType = as<IRArrayTypeBase>(transformedValueType))
        {
            auto targetArrayType = as<IRArrayTypeBase>(targetOriginalType);
            if (targetArrayType)
            {
                auto transformedElementType = transformedArrayType->getElementType();
                auto targetElementType = targetArrayType->getElementType();
                
                // Check if the array's element type corresponds to a transformed type
                IRType* expectedTransformedElementType = nullptr;
                if (originalToLoweredTypeMap.tryGetValue(
                        targetElementType,
                        expectedTransformedElementType) &&
                    expectedTransformedElementType == transformedElementType)
                {
                    // We need to convert each element of the array from transformed type to original type
                    
                    // Get the array count - handle both sized and unsized arrays
                    auto arrayCount = transformedArrayType->getElementCount();
                    if (!arrayCount)
                    {
                        // For unsized arrays, we can't convert element by element at this level
                        // The conversion should happen at the access level instead
                        return transformedValue;
                    }
                    
                    // Extract array count as integer literal
                    auto arrayCountLit = as<IRIntLit>(arrayCount);
                    if (!arrayCountLit)
                    {
                        // For non-constant array sizes, we can't convert element by element
                        // The conversion should happen at the access level instead
                        return transformedValue;
                    }
                    
                    // Convert each array element
                    List<IRInst*> convertedElements;
                    IntegerLiteralValue count = arrayCountLit->getValue();
                    
                    for (IntegerLiteralValue i = 0; i < count; ++i)
                    {
                        // Create index constant
                        auto indexInst = irBuilder.getIntValue(irBuilder.getIntType(), i);
                        
                        // Extract the element at index i from the transformed array
                        auto transformedElement = irBuilder.emitElementExtract(
                            transformedValue, 
                            indexInst);
                        
                        // Convert the extracted element from transformed type to original type
                        auto convertedElement = convertDescriptorHandleToOriginalType(
                            transformedElement, 
                            targetElementType);
                        
                        convertedElements.add(convertedElement);
                    }
                    
                    // Create a new array with the converted elements
                    return irBuilder.emitMakeArray(targetOriginalType, convertedElements.getCount(), convertedElements.getBuffer());
                }
            }
        }

        // No conversion needed - return value as-is
        return transformedValue;
    }

    /// Helper function to convert a transformed instruction to original type and replace its use
    ///
    /// This function performs the common pattern of loading a transformed value, converting it
    /// back to the original type, storing it in new storage, and replacing the use with the
    /// new storage pointer. It also tracks processed instructions to avoid infinite loops.
    ///
    /// @param currentInst The instruction with transformed type to convert
    /// @param userInstruction The instruction that will use the converted value
    /// @param use The specific use to replace
    /// @param processed HashSet to track processed instructions for loop prevention
    /// @return true if conversion was performed, false otherwise
    bool performTransformedToOriginalConversionForUse(
        IRInst* currentInst, 
        IRInst* userInstruction, 
        IRUse* use, 
        HashSet<IRInst*>& processed)
    {
        auto currentInstType = currentInst->getFullType();
        
        IRType* transformedType = nullptr;
        IRType* originalType = nullptr;

        // Check if current instruction has PtrType<TransformedType>
        if (auto ptrType = as<IRPtrTypeBase>(currentInstType))
        {
            transformedType = ptrType->getValueType();
        }
        // Check if current instruction is a ParameterBlock<TransformedType>
        else if (auto paramBlockType = as<IRParameterBlockType>(currentInstType))
        {
            transformedType = paramBlockType->getElementType();
        }

        // Check if we have a mapping from transformed type to original type
        if (!transformedType || !loweredToOriginalTypeMap.tryGetValue(transformedType, originalType))
        {
            return false; // No conversion needed
        }

        // Insert conversion instructions before the user instruction
        irBuilder.setInsertBefore(userInstruction);
        
        // Load the transformed value from the source
        auto transformedValue = irBuilder.emitLoad(currentInst);
        
        // Convert the transformed value back to original type
        auto convertedValue = convertDescriptorHandleToOriginalType(
            transformedValue, 
            originalType);
        
        // Create storage for the converted value with original type
        auto convertedPtr = irBuilder.emitVar(originalType);
        auto store = irBuilder.emitStore(convertedPtr, convertedValue);

        // Track the newly created instructions to avoid processing them again
        processed.add(transformedValue);
        processed.add(convertedPtr);
        processed.add(store);

        // Replace the use of currentInst with convertedPtr
        use->set(convertedPtr);

        return true; // Conversion was performed
    }

        void updateTypesAndInsertCast(IRInst* startInst) {
            
            List<IRInst*> worklist;
            HashSet<IRInst*> processed; // Track processed instructions to avoid infinite loops
            
            worklist.add(startInst);

            while (worklist.getCount() > 0) {
                auto currentInst = worklist.getLast();
                worklist.removeLast();
                // Skip if already processed
                if (processed.contains(currentInst)) {
                    continue;
                }
                processed.add(currentInst);

                // Process all users of the current instruction
                for (auto use = currentInst->firstUse; use; use = use->nextUse)
                {
                    auto userInstruction = use->getUser();
                    if (processed.contains(userInstruction)) {
                        continue;
                    }
                    switch (userInstruction->getOp())
                    {
                    case kIROp_FieldAddress:
                    case kIROp_GetElementPtr:
                    {
                        // Update pointer types for field/element addresses to point to transformed types
                        auto currentPtrType = as<IRPtrTypeBase>(userInstruction->getFullType());
                        if (!currentPtrType) break;
                        
                        auto originalValueType = currentPtrType->getValueType();
                        IRType* transformedValueType = nullptr;
                        
                        if (originalToLoweredTypeMap.tryGetValue(originalValueType, transformedValueType))
                        {
                            // Create new pointer type pointing to the transformed type
                            auto transformedPtrType = irBuilder.getPtrTypeWithAddressSpace(
                                transformedValueType, // new value type (with descriptor handles)
                                currentPtrType);      // preserves pointer op and address space
                            
                            // Update the instruction's type
                            userInstruction->setFullType(transformedPtrType);
                        }
                        
                        // Add to worklist for further processing
                        worklist.add(userInstruction);
                    }
                    break;
                    case kIROp_Call:
                    {
                        // Handle function calls - check if the function expects transformed types
                        auto callInst = as<IRCall>(userInstruction);
                        auto calleeFunc = as<IRFunc>(callInst->getCallee());
                        if (!calleeFunc)
                        {
                            worklist.add(userInstruction);
                            break;
                        }

                        auto currentInstType = currentInst->getFullType();
                        IRType* transformedType = nullptr;

                        // Check if current instruction has PtrType<TransformedType>
                        if (auto ptrType = as<IRPtrTypeBase>(currentInstType))
                        {
                            transformedType = ptrType->getValueType();
                        }
                        // Check if current instruction is a ParameterBlock<TransformedType>
                        else if (auto paramBlockType = as<IRParameterBlockType>(currentInstType))
                        {
                            transformedType = paramBlockType->getElementType();
                        }

                        // Check if the function has parameters that expect transformed types
                        // Only ParameterBlockType is likely to be transformed in function params
                        bool functionExpectsTransformedTypes = false;
                        for (auto param = calleeFunc->getFirstParam(); param; param = param->getNextParam())
                        {
                            auto paramType = param->getDataType();

                            // Check if this parameter type is a transformed ParameterBlock type
                            if (auto paramBlockType = as<IRParameterBlockType>(paramType))
                            {
                                auto elementType = paramBlockType->getElementType();
                                if (elementType == transformedType)
                                {
                                    functionExpectsTransformedTypes = true;
                                    break;
                                }
                            }
                        }

                        // If function expects transformed types, keep the transformed type
                        if (functionExpectsTransformedTypes)
                        {
                            // No conversion needed - the function expects the transformed type
                            // Just add to worklist for further processing
                            worklist.add(userInstruction);
                        }
                        else
                        {
                            // Function expects original types, so convert back
                            // Use helper function to convert transformed types to original types
                            performTransformedToOriginalConversionForUse(currentInst, userInstruction, use, processed);

                            // Add to worklist for further processing
                            worklist.add(userInstruction);
                        }
                    
                        break;
                    }

                    default:
                    {
                        // Use helper function to convert transformed types to original types
                        performTransformedToOriginalConversionForUse(currentInst, userInstruction, use, processed);
                        // For other instruction types, just add to worklist
                        worklist.add(userInstruction);
                    }
                    break;
                    }
                }
            }
        }

        void updateTypesAndInsertCast() {
            for (auto transformedParameterBlockType : updatedParameterBlockTypes)
            {
                for (auto use = transformedParameterBlockType->firstUse; use; use = use->nextUse)
                {
                    auto userInstruction = use->getUser();
                    if (userInstruction->getOp() == kIROp_GlobalParam || 
                        userInstruction->getOp() == kIROp_Param ||
                        userInstruction->getOp() == kIROp_FieldExtract)
                    {
                        // Begin top-down processing from this global parameter or function parameter
                        updateTypesAndInsertCast(userInstruction);
                    }
                }
            }
        }

        /// Main entry point for processing the entire module
        ///
        /// Orchestrates the complete descriptor handle transformation:
        /// 1. Transforms parameter block types (including RWStructuredBuffer wrapping)
        /// 2. Applies transformations using top-down approach
        void processModule()
        {
            transformParameterBlockTypes();
            updateTypesAndInsertCast();
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
