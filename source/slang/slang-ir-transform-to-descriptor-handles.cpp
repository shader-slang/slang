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

    // Track FieldAddress instructions that now return descriptor handles
    List<IRInst*> descriptorHandleFieldAccessList;
    Dictionary<IRInst*, IRInst*> fieldAccessToField;

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


    void processLoadOperationsForFieldAccess()
    {
        // For each fieldAccess (fieldExtract or fieldAddress)
        // that now returns descriptor handle or ptr to descriptor handle
        for (auto fieldAccess : descriptorHandleFieldAccessList)
        {
            auto field = fieldAccessToField[fieldAccess];

            if (auto fieldExtract = as<IRFieldExtract>(fieldAccess))
            {
                // this fieldExtract returns a handler now, but its users still expect a resource
                // type insert a cast inst. after fieldExtract to convert the descriptor handle to
                // resource type and update the user inst. to use this cast inst.
                builder.setInsertAfter(fieldExtract);
                auto fieldType = fieldExtract->getFullType();


                if (auto descriptorHandleType = as<IRDescriptorHandleType>(fieldType))
                {
                    auto castInst = builder.emitIntrinsicInst(
                        descriptorHandleType->getResourceType(),
                        kIROp_CastDescriptorHandleToResource,
                        1,
                        &field);
                    // TODO: how to update the users of fieldExtract?
                    // for (auto use = fieldExtract->firstUse; use; use = use->nextUse)
                    //{
                    //    use->getUser()->setOperand(0, castInst);
                    //}
                }
            }
            else if (auto fieldAddress = as<IRFieldAddress>(fieldAccess))
            {

                // Load (user of fieldAddress) now returns a pointer to descriptor handle
                // the result of Load should be casted back to the resource type.

                auto ptrType = fieldAddress->getFullType();
                auto fieldType = as<IRPtrTypeBase>(ptrType)->getOperand(0);
                if (auto descriptorHandleType = as<IRDescriptorHandleType>(fieldType))
                {
                    auto castInst = builder.emitIntrinsicInst(
                        descriptorHandleType->getResourceType(),
                        kIROp_CastDescriptorHandleToResource,
                        1,
                        &field);

                    for (auto use = fieldAddress->firstUse; use; use = use->nextUse)
                    {
                        if (auto load = as<IRLoad>(use->getUser()))
                        {
                            // TODO: How to update the user of fieldAddress?
                        }
                    }
                }
            }
        }
    }
    void processParameterBlockType(IRParameterBlockType* paramBlockType)
    {
        // TODO: make a clone of the struct type inside parameter block to avoid modifying the
        // original type.
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
        } // else {} TODO: other forms of nested struct inside parameter block?

        // Second mini-pass: Find operations that target these FieldAddress or FieldExtract
        // instructions
        processLoadOperationsForFieldAccess();
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
                        //// Track this field extract for Load processing
                        // TODO
                        descriptorHandleFieldAccessList.add(fieldExtract);
                        fieldAccessToField[fieldExtract] = field;
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

                        // Track this field address for Load processing
                        descriptorHandleFieldAccessList.add(fieldAddress);
                        fieldAccessToField[fieldAddress] = field;
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
