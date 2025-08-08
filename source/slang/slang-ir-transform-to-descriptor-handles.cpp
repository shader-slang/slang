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
    
    // Track cloned structs and their field mapping for conversion logic
    Dictionary<IRStructType*, IRStructType*> clonedStructMap;

    ResourceToDescriptorHandleContext(TargetProgram* inTargetProgram, IRModule* module, DiagnosticSink* inSink)
        : InstPassBase(module)
        , targetProgram(inTargetProgram)
        , sink(inSink)
        , builder(module)
    {
    }

    // Recursive type lowering function
    IRType* convertTypeToHandler(IRType* originalType)
    {
        // Check if we've already lowered this type
        if (auto existing = typeLoweringMap.tryGetValue(originalType))
            return *existing;

        IRType* loweredType = nullptr;

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
                // Clone the struct with lowered field types
                builder.setInsertBefore(structType);
                auto newStructType = builder.createStructType();
                
                // Copy decorations from original
                copyNameHintAndDebugDecorations(newStructType, structType);
                
                // derive a distinct name
                StringBuilder sb;
                if (auto nameDeco = structType->findDecoration<IRNameHintDecoration>())
                {
                    auto name = as<IRStringLit>(nameDeco->getOperand(0))->getStringSlice();
                    sb << name << "_new";

                    // Create the string literal IR inst
                    auto strLit = builder.getStringValue(sb.getUnownedSlice());

                    // Option A: update existing name decoration
                    if (auto nameDeco = newStructType->findDecoration<IRNameHintDecoration>())
                    {
                        nameDeco->setOperand(0, strLit);
                    }
                }
                
                // Clone all fields with potentially lowered types
                for (auto originalField : structType->getFields())
                {
                    auto fieldType = originalField->getFieldType();
                    auto loweredFieldType = convertTypeToHandler(fieldType);
                    auto fieldKey = originalField->getKey();
                    
                    auto newField = builder.createStructField(newStructType, fieldKey, loweredFieldType);

                }
                
                loweredType = newStructType;
                clonedStructMap[structType] = newStructType;
            }
            else
            {
                // No lowering needed
                loweredType = originalType;
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
            else
            {
                loweredType = originalType;
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
            else
            {
                loweredType = originalType;
            }
        }
        else
        {
            // No lowering needed for this type
            loweredType = originalType;
        }

        // Cache the result
        typeLoweringMap[originalType] = loweredType;

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
            builder.setInsertAfter(paramBlockType);
            auto newParamBlockType = builder.getType(kIROp_ParameterBlockType, loweredElementType);

                        paramBlockType->replaceUsesWith(newParamBlockType);
            paramBlockType->removeAndDeallocate();
        }
    }

    void processModule()
    {
        cloneParameterBlockStructs();


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
