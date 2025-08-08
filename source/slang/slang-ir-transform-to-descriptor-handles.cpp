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

    ResourceToDescriptorHandleContext(TargetProgram* inTargetProgram, IRModule* module, DiagnosticSink* inSink)
        : InstPassBase(module)
        , targetProgram(inTargetProgram)
        , sink(inSink)
        , builder(module)
    {
    }

    IRType* createDescriptorHandleType(IRType* resourceType)
    {
        builder.setInsertBefore(resourceType);
        
        // Create DescriptorHandle<ResourceType>
        return (IRDescriptorHandleType*)builder.getType(kIROp_DescriptorHandleType, resourceType);
    }

    IRInst* createCastDescriptorHandleToResource(IRType* resourceType, IRInst* descriptorHandle)
    {
        builder.setInsertBefore(descriptorHandle);
        return builder.emitIntrinsicInst(
            resourceType,
            kIROp_CastDescriptorHandleToResource,
            1,
            &descriptorHandle);
    }

    void processParameterBlockType(IRParameterBlockType* paramBlockType)
    {
        auto elementType = paramBlockType->getElementType();
        if (auto structType = as<IRStructType>(elementType))
        {
            processStructTypeInParameterBlock(structType);
        }
        else if (auto arrayType = as<IRArrayTypeBase>(elementType))
        {
            if (auto innerArrayStructType = as<IRStructType>(arrayType->getElementType()))
            {
                processStructTypeInParameterBlock(innerArrayStructType);
            }
        }
    }

    void processStructTypeInParameterBlock(IRStructType* structType)
    {
        for (auto field : structType->getFields())
        {
            auto fieldType = field->getFieldType();
            if (auto innerStructType = as<IRStructType>(fieldType))
            {
                processStructTypeInParameterBlock(innerStructType);
            }
            else if (auto arrayType = as<IRArrayTypeBase>(fieldType))
            {
                if (auto innerArrayStructType = as<IRStructType>(arrayType->getElementType()))
                {
                    processStructTypeInParameterBlock(innerArrayStructType);
                }
            }
            if (isResourceType(fieldType))
            {
                auto descriptorHandleType = createDescriptorHandleType(fieldType);
                field->setFieldType(descriptorHandleType);

                // field itself has no uses; we need to look at all uses of the field's StructKey inst.
                auto structKey = field->getKey();
                for (auto use = structKey->firstUse; use; use = use->nextUse)
                {
                    //  If the use is a FieldAddress or FieldExtract, update the type of the inst accordingly.
                    if (auto fieldExtract = as<IRFieldExtract>(use->getUser()))
                    {
                        fieldExtract->setFullType(descriptorHandleType);
                    }
                    else if (auto fieldAddress = as<IRFieldAddress>(use->getUser()))
                    {
                        auto currentPtrType = as<IRPtrTypeBase>(fieldAddress->getFullType());
                        auto ptrOp = currentPtrType->getOp();

                        // Create new pointer type pointing to descriptor handle
                        auto descriptorHandlePtrType = builder.getPtrTypeWithAddressSpace(
                            descriptorHandleType,    // new value type
                            currentPtrType);         // preserves op + address space

                        // Update the field address type
                        fieldAddress->setFullType(descriptorHandlePtrType);
                    }
                }
            }
        }
    }

    void processModule()
    {
        for (auto globalInst : module->getGlobalInsts())
        {
            if (auto paramBlockType = as<IRParameterBlockType>(globalInst))
            {
                processParameterBlockType(paramBlockType);
            }
        }


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
