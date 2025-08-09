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
    Dictionary<IRType*, IRType*> typeLoweringMap;

    // Track which fields have been updated to descriptor handles
    // for updating the instructions that access these fields
    List<IRStructField*> updatedFields;

    ResourceToDescriptorHandleContext(
        TargetProgram* inTargetProgram,
        IRModule* module,
        DiagnosticSink* inSink)
        : InstPassBase(module), targetProgram(inTargetProgram), sink(inSink), builder(module)
    {
    }

    void updateInstructionsAccessingTheField(IRStructField* field, IRType* loweredFieldType)
    {
        // field itself has no uses; we need to look at all uses of the field's StructKey inst.
        auto structKey = field->getKey();
        for (auto use = structKey->firstUse; use; use = use->nextUse)
        {
            auto user = use->getUser();
            if (auto fieldAddress = as<IRFieldAddress>(user))
            {
                // If this is a field address, we need to the ptrType
                auto currentPtrType = as<IRPtrTypeBase>(fieldAddress->getFullType());

                // Create new pointer type pointing to descriptor handle
                auto descriptorHandlePtrType = builder.getPtrTypeWithAddressSpace(
                    loweredFieldType, // new value type
                    currentPtrType);  // preserves op + address space

                // Update the field address type
                fieldAddress->setFullType(descriptorHandlePtrType);
            }
            else if (auto fieldExtract = as<IRFieldExtract>(user))
            {
                fieldExtract->setFullType(loweredFieldType);
            }
            else if (auto getElementPtr = as<IRGetElement>(user))
            {
                // If this is a field address, we need to the ptrType
                auto currentPtrType = as<IRPtrTypeBase>(getElementPtr->getFullType());

                // Create new pointer type pointing to descriptor handle
                auto descriptorHandlePtrType = builder.getPtrTypeWithAddressSpace(
                    loweredFieldType, // new value type
                    currentPtrType);  // preserves op + address space

                // Update the field address type
                getElementPtr->setFullType(descriptorHandlePtrType);
            }
        }
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

                // Set distinct name
                if (auto nameHint = structType->findDecoration<IRNameHintDecoration>())
                {
                    // Get the original name string
                    auto originalNameString = as<IRStringLit>(nameHint->getOperand(0));
                    if (originalNameString)
                    {
                        // Create new name by appending "_new"
                        String newName = originalNameString->getStringSlice();
                        newName.append("_new");

                        // Add the new name hint decoration
                        builder.addNameHintDecoration(newStructType, newName.getUnownedSlice());
                    }
                }

                // Clone all fields with potentially lowered types
                for (auto originalField : structType->getFields())
                {
                    auto fieldType = originalField->getFieldType();
                    auto loweredFieldType = convertTypeToHandler(fieldType);
                    auto fieldKey = originalField->getKey();

                    auto newField =
                        builder.createStructField(newStructType, fieldKey, loweredFieldType);

                    updateInstructionsAccessingTheField(newField, loweredFieldType);
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
    void processAddressChain(IRInst* accessInst, IRStructField* field)
    {
        // Process certain users of this address instruction
        for (auto use = accessInst->firstUse; use; use = use->nextUse)
        {
            auto user = use->getUser();

            switch (user->getOp())
            {
            case kIROp_FieldAddress:
            case kIROp_GetElementPtr:
                // These are address computations - follow through recursively
                processAddressChain(user, field);
                break;

            case kIROp_Load:
                {
                    // Load now should load a descriptor handle ptr type from FiedlAddress
                    auto type = field->getFieldType();
                    user->setFullType(type);

                    if (auto descriptorHandleType = as<IRDescriptorHandleType>(type))
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
    void insertConversionLogic()
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

            processAddressChain(structKey, field);
        }
    }

    void processModule()
    {
        cloneParameterBlockStructs();
        insertConversionLogic();
    }
};

void transformResourceTypesToDescriptorHandles(
    TargetProgram* targetProgram,
    IRModule* module,
    DiagnosticSink* sink)
{
    ResourceToDescriptorHandleContext context(targetProgram, module, sink);
    context.processModule();

    // Dump IR after transform
    IRDumpOptions opt;
    opt.mode = IRDumpOptions::Mode::Detailed; // or Simplified
    opt.flags = IRDumpOptions::Flag::DumpDebugIds;

    DiagnosticSinkWriter writer(sink);
    dumpIR(module, opt, "AFTER-DESC-HANDLE-TRANSFORM", nullptr, &writer);
    }

}
