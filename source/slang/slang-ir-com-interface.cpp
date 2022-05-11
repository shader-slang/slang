// slang-ir-com-interface.cpp
#include "slang-ir-com-interface.h"

#include "slang-ir.h"
#include "slang-ir-insts.h"

namespace Slang
{

struct ComInterfaceLoweringContext
{
    IRModule* module;
    DiagnosticSink* diagnosticSink;

    SharedIRBuilder sharedBuilder;

    Dictionary<IRInterfaceType*, IRComPtrType*> comPtrTypes;

    void replaceTypeUses(IRInst* inst, IRInst* newValue)
    {
        List<IRUse*> uses;
        for (auto use = inst->firstUse; use; use = use->nextUse)
        {
            uses.add(use);
        }
        for (auto use : uses)
        {
            switch (use->getUser()->getOp())
            {
            case kIROp_WitnessTableIDType:
            case kIROp_WitnessTableType:
            case kIROp_ThisType:
            case kIROp_RTTIPointerType:
            case kIROp_RTTIHandleType:
            case kIROp_ComPtrType:
                continue;
            default:
                break;
            }
            use->set(newValue);
        }
    }

    IRComPtrType* processInterfaceType(IRInterfaceType* type)
    {
        if (!type->findDecoration<IRComInterfaceDecoration>())
            return nullptr;

        IRComPtrType* result = nullptr;

        if (comPtrTypes.TryGetValue(type, result))
            return result;

        IRBuilder builder(sharedBuilder);
        builder.setInsertInto(module->getModuleInst());
        result = builder.getComPtrType(type);

        replaceTypeUses(type, result);
        return result;
    }

    void processThisType(IRThisType* type)
    {
        auto comPtrType = processInterfaceType(as<IRInterfaceType>(type->getConstraintType()));
        if (!comPtrType)
            return;
        replaceTypeUses(type, comPtrType);
    }

    void processModule()
    {
        for (auto child : module->getGlobalInsts())
        {
            switch (child->getOp())
            {
            case kIROp_InterfaceType:
                processInterfaceType(as<IRInterfaceType>(child));
                break;
            case kIROp_ThisType:
                processThisType(as<IRThisType>(child));
                break;
            default:
                break;
            }
        }
    }
};

void lowerComInterfaces(IRModule* module, DiagnosticSink* sink)
{
    ComInterfaceLoweringContext context;
    context.module = module;
    context.diagnosticSink = sink;
    context.sharedBuilder.init(module);
    return context.processModule();
}

}
