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
                    
                    case kIROp_Load:
                    case kIROp_Call:
                    {
                        // Insert conversion before the load or call
                        // so that the Load/Call operates on the original type data
                        auto currentInstType = currentInst->getFullType();
                        
                        IRType* transformedType = nullptr;
                        IRType* originalType = nullptr;
                        IRInst* sourceToLoad = nullptr;
                        
                        // Check if current instruction has PtrType<TransformedType>
                        if (auto ptrType = as<IRPtrTypeBase>(currentInstType))
                        {
                            auto pointeeType = ptrType->getValueType();
                            if (loweredToOriginalTypeMap.tryGetValue(pointeeType, originalType))
                            {
                                transformedType = pointeeType;
                                sourceToLoad = currentInst;  // Load from the pointer
                            }
                        }
                        // Check if current instruction is a ParameterBlock<TransformedType>
                        else if (auto paramBlockType = as<IRParameterBlockType>(currentInstType))
                        {
                            auto elementType = paramBlockType->getElementType();
                            if (loweredToOriginalTypeMap.tryGetValue(elementType, originalType))
                            {
                                transformedType = elementType;
                                sourceToLoad = currentInst;  // Load from the parameter block
                            }
                        }
                        
                        // If we found a transformed type that needs conversion
                        if (transformedType && originalType && sourceToLoad)
                        {
                            // Insert conversion instructions before the load so it can operate normally
                            irBuilder.setInsertBefore(userInstruction);
                            
                            // Load the transformed value from the source
                            auto transformedValue = irBuilder.emitLoad(sourceToLoad);
                            
                            // Convert the transformed value back to original type
                            auto convertedValue = convertDescriptorHandleToOriginalType(
                                transformedValue, 
                                originalType);
                            
                            // Create storage for the converted value with original type
                            auto convertedPtr = irBuilder.emitVar(originalType);
                            irBuilder.emitStore(convertedPtr, convertedValue);
                            
                            // Replace the use of currentInst in the load with convertedPtr
                            // This makes the load operate on the original type pointer/source
                            use->set(convertedPtr);
                        }
                        // Add the user instruction to worklist for further processing
                        worklist.add(userInstruction);
                    }
                    break;
                    
                    default:
                    {
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
                    if (auto globalParameterVariable = as<IRGlobalParam>(userInstruction))
                    {
                        // Begin top-down processing from this global parameter
                        updateTypesAndInsertCast(globalParameterVariable);
                    }
                }
            }
        }

        /// Main entry point for processing the entire module
        ///
        /// Orchestrates the complete descriptor handle transformation:
        /// 1. Transforms parameter block types to use descriptor handles
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
