// slang-ir-dll-export.cpp
#include "slang-ir-dll-export.h"

#include "slang-ir.h"
#include "slang-ir-insts.h"
#include "slang-ir-marshal-native-call.h"

namespace Slang
{

struct DllExportContext
{
    IRModule* module;
    DiagnosticSink* diagnosticSink;

    void processFunc(IRFunc* func, IRDllExportDecoration* dllExportDecoration)
    {
        NativeCallMarshallingContext marshalContext;
        marshalContext.diagnosticSink = diagnosticSink;
        IRBuilder builder(module);
        auto wrapper = marshalContext.generateDLLExportWrapperFunc(builder, func);
        dllExportDecoration->removeFromParent();
        dllExportDecoration->insertAtStart(wrapper);
        builder.addNameHintDecoration(wrapper, dllExportDecoration->getFunctionName());
        builder.addExternCppDecoration(wrapper, dllExportDecoration->getFunctionName());
        builder.addPublicDecoration(wrapper);
        builder.addKeepAliveDecoration(wrapper);
        builder.addHLSLExportDecoration(wrapper);
        if (auto oldPublicDecoration = func->findDecoration<IRPublicDecoration>())
        {
            oldPublicDecoration->removeFromParent();
        }
    }

    void processModule()
    {
        struct Candidate { IRFunc* func; IRDllExportDecoration* exportDecoration; };
        List<Candidate> candidates;
        for (auto childFunc : module->getGlobalInsts())
        {
            switch(childFunc->getOp())
            {
            case kIROp_Func:
                if (auto dllExportDecoration = childFunc->findDecoration<IRDllExportDecoration>())
                {
                    candidates.add(Candidate{ as<IRFunc>(childFunc), dllExportDecoration });
                }
                break;
            default:
                break;
            }
        }

        for (auto candidate : candidates)
        {
            processFunc(candidate.func, candidate.exportDecoration);
        }
    }
};

void generateDllExportFuncs(IRModule* module, DiagnosticSink* sink)
{
    DllExportContext context;
    context.module = module;
    context.diagnosticSink = sink;
    return context.processModule();
}

}
