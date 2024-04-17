// slang-ir-header-export.cpp
#include "slang-ir-header-export.h"

#include "slang-ir.h"
#include "slang-ir-insts.h"

namespace Slang
{

static void markForHeaderExport_(IRBuilder & builder, IRInst * inst)
{
    if (auto func = as<IRFunc>(inst))
    {
        if (!func->findDecoration<IRHeaderExportDecoration>())
        {    
            builder.addHeaderExportDecoration(func);
        }
        for (auto param : func->getFirstBlock()->getParams())
        {
            markForHeaderExport_(builder, param->getDataType());
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
            IRBuilder builder(structType->getModule());

            // If it already has a header-export decoration, we're done.
            if (!structType->findDecoration<IRHeaderExportDecoration>())
            {    
                builder.addHeaderExportDecoration(structType);
            }

            for (auto field : structType->getFields())
            {
                markForHeaderExport_(builder, field->getFieldType());
            }
            return;
        }
        else if (auto arrayType = as<IRArrayType>(type))
        {
            // TODO: not sure about this
            IRBuilder builder(arrayType->getModule());
            if (!arrayType->findDecoration<IRHeaderExportDecoration>())
                builder.addHeaderExportDecoration(arrayType);

            markForHeaderExport_(builder, arrayType->getElementType());
            return;
        }
    }
}

struct HeaderExportContext 
{
    IRModule* module;
    DiagnosticSink* diagnosticSink;

    void processInst(IRInst* inst, IRHeaderExportDecoration* headerExportDecoration)
    {
        IRBuilder builder(module);
        // TODO: required? builder.addNameHintDecoration(inst, headerExportDecoration->getFunctionName());
        // TODO: required? builder.addExternCppDecoration(inst, headerExportDecoration->getFunctionName());
        markForHeaderExport_(builder, inst);
        // TODO: required? builder.addPublicDecoration(inst);
        // TODO: required? builder.addKeepAliveDecoration(inst);
        // TODO: required? builder.addHLSLExportDecoration(inst);
    }

    void processModule()
    {
        struct Candidate { IRInst* inst; IRHeaderExportDecoration* exportDecoration; };
        List<Candidate> candidates;
        for (auto childInst : module->getGlobalInsts())
        {
            switch(childInst->getOp())
            {
            case kIROp_Func:
            case kIROp_StructType:
                if (auto headerExportDecoration = childInst->findDecoration<IRHeaderExportDecoration>())
                {
                    candidates.add(Candidate{ as<IRInst>(childInst), headerExportDecoration });
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

void generateTransitiveHeaderExports(IRModule* module, DiagnosticSink* sink)
{
    HeaderExportContext context;
    context.module = module;
    context.diagnosticSink = sink;
    return context.processModule();
}

}
