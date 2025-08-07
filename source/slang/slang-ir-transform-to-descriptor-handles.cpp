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
        builder.setInsertBefore(descriptorHandle->getNextInst());
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

                // The use of field now returns a descriptor handle, but users expect the resource
                // type. Insert a cast instruction after this field extract
                auto castInst =
                    createCastDescriptorHandleToResource(fieldType, descriptorHandleType);

                // Replace all uses of the field with the cast instruction
                for (auto use = field->firstUse; use; use = use->nextUse)
                {
                    // Don't replace the use in the cast instruction itself
                    if (use->getUser() != castInst)
                    {
                        use->set(castInst);
                    }
                }
            }
        }
    }

    void processModule()
    {
        // First pass: Process parameter block types and transform struct field types within them
        processInstsOfType<IRParameterBlockType>(
            kIROp_ParameterBlockType,
            [this](IRParameterBlockType* paramBlockType)
            { processParameterBlockType(paramBlockType); });
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
