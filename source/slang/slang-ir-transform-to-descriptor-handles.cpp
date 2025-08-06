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

    void processStructType(IRStructType* structType)
    {
        for (auto field : structType->getFields())
        {
            if (isResourceType(field->getFieldType()))
            {
                auto fieldType = field->getFieldType();
                auto descriptorHandleType = createDescriptorHandleType(fieldType);
                field->setFieldType(descriptorHandleType);
            }
        }
    }

    void processModule()
    {
        // Process all struct types in the module
        processInstsOfType<IRStructType>(kIROp_StructType, 
            [this](IRStructType* structType)
        {
            processStructType(structType);
        });
    }
};

void transformResourceTypesToDescriptorHandles(
    TargetProgram* targetProgram, 
    IRModule* module, 
    DiagnosticSink* sink)
{
    // Only apply this transformation for Metal targets
    if (!isMetalTarget(targetProgram->getTargetReq()))
        return;

    ResourceToDescriptorHandleContext context(targetProgram, module, sink);
    context.processModule();
}

} 
