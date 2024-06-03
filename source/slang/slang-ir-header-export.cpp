// slang-ir-header-export.cpp
#include "slang-ir-header-export.h"

#include "slang-ir.h"
#include "slang-ir-insts.h"

namespace Slang
{

static void addExternCppDecorationTransitively_(IRBuilder & builder, IRInst * inst)
{
    // addExternCppDecoration transitively for function params and struct members (nested)
    
    if (auto func = as<IRFunc>(inst))
    {
        if (!func->findDecoration<IRExternCppDecoration>())
        {    
            builder.addExternCppDecoration(func, UnownedStringSlice(""));
        }
        for (auto param : func->getFirstBlock()->getParams())
        {
            addExternCppDecorationTransitively_(builder, param->getDataType());
        }
    }
    else if (auto type = as<IRType>(inst))
    {
        // If it's a basic type, we're done.
        if (as<IRBasicType>(type) || as<IRVoidType>(type))
            return;
        
        // If it's a struct type, mark for py-export.
        if (auto structType = as<IRStructType>(type))
        {
            IRBuilder structBuilder(structType->getModule());

            // If it already has a header-export decoration, we're done.
            if (!structType->findDecoration<IRExternCppDecoration>())
            {    
                structBuilder.addExternCppDecoration(structType, UnownedStringSlice(""));
            }

            for (auto field : structType->getFields())
            {
                addExternCppDecorationTransitively_(structBuilder, field->getFieldType());
            }
            return;
        }
        else if (auto arrayType = as<IRArrayType>(type))
        {
            // TODO: not sure about this
            IRBuilder arrayBuilder(arrayType->getModule());
            if (!arrayType->findDecoration<IRExternCppDecoration>())
                arrayBuilder.addExternCppDecoration(arrayType, UnownedStringSlice(""));

            addExternCppDecorationTransitively_(arrayBuilder, arrayType->getElementType());
            return;
        }
    }
}

struct HeaderExportContext 
{
    IRModule* module;
    DiagnosticSink* diagnosticSink;

    void processInst(IRInst* inst, IRExternCppDecoration* /*externCppDecoration*/)
    {
        IRBuilder builder(module);
        addExternCppDecorationTransitively_(builder, inst);
        // TODO: required? builder.addNameHintDecoration(inst, externCppDecoration->getFunctionName());
        // TODO: required? builder.addExternCppDecoration(inst, externCppDecoration->getFunctionName());
        // TODO: required? builder.addPublicDecoration(inst);
        // TODO: required? builder.addKeepAliveDecoration(inst);
        // TODO: required? builder.addHLSLExportDecoration(inst);
    }

    void processModule()
    {
        struct Candidate { IRInst* inst; IRExternCppDecoration* exportDecoration; };
        List<Candidate> candidates;
        for (auto childInst : module->getGlobalInsts())
        {
            switch(childInst->getOp())
            {
            case kIROp_Func:
            case kIROp_StructType:
                if (auto externCppDecoration = childInst->findDecoration<IRExternCppDecoration>())
                {
                    candidates.add(Candidate{ as<IRInst>(childInst), externCppDecoration });
                }
                break;
            default:
                break;
            }
        }

        for (auto candidate : candidates)
        {
            processInst(candidate.inst, candidate.exportDecoration);
        }
    }
};

void generateTransitiveExternCpp(IRModule* module, DiagnosticSink* sink)
{
    HeaderExportContext context;
    context.module = module;
    context.diagnosticSink = sink;
    return context.processModule();
}

}
