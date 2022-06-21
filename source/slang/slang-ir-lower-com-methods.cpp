// slang-ir-lower-com-methods.cpp

#include "slang-ir-lower-com-methods.h"

#include "slang-ir.h"
#include "slang-ir-insts.h"
#include "slang-ir-marshal-native-call.h"
#include "slang-ir-inst-pass-base.h"

namespace Slang
{

struct ComMethodLoweringContext : public InstPassBase
{
    DiagnosticSink* diagnosticSink = nullptr;

    NativeCallMarshallingContext marshal;

    OrderedHashSet<IRLookupWitnessMethod*> comCallees;

    ComMethodLoweringContext(IRModule* inModule)
        : InstPassBase(inModule)
    {}

    void processComCall(IRCall* comCall)
    {
        IRBuilder builder(&sharedBuilderStorage);
        builder.setInsertBefore(comCall);
        auto callee = as<IRLookupWitnessMethod>(comCall->getCallee());
        SLANG_ASSERT(callee);

        comCallees.Add(callee);
        
        auto calleeType = as<IRFuncType>(comCall->getCallee()->getDataType());
        SLANG_ASSERT(calleeType);

        auto nativeFuncType = marshal.getNativeFuncType(builder, calleeType);
        ShortList<IRInst*> args;
        for (UInt i = 0; i < comCall->getArgCount(); i++)
            args.add(comCall->getArg(i));
        auto currentBlock = builder.getBlock();
        auto nextInst = comCall->getNextInst();
        auto newResult = marshal.marshalNativeCall(
            builder,
            calleeType,
            nativeFuncType,
            comCall->getCallee(),
            args.getCount(),
            args.getArrayView().getBuffer());

        comCall->replaceUsesWith(newResult);
        if (builder.getBlock() != currentBlock)
        {
            // `marshalNativeCall` may have replaced the original call with branch insts.
            // If this is the case, we need to move all insts after the original call in the original
            // basic block to the new basic block.
            while (nextInst)
            {
                auto next = nextInst->getNextInst();
                nextInst->removeFromParent();
                nextInst->insertAtEnd(builder.getBlock());
                nextInst = next;
            }
        }
        comCall->removeAndDeallocate();
    }

    void processCall(IRCall* inst)
    {
        auto funcValue = inst->getOperand(0);

        // Detect if this is a call into a COM interface method.
        if (funcValue->getOp() == kIROp_lookup_interface_method)
        {
            const auto operand0TypeOp = funcValue->getOperand(0)->getDataType();
            if (auto tableType = as<IRWitnessTableTypeBase>(operand0TypeOp))
            {
                if (tableType->getConformanceType()->findDecoration<IRComInterfaceDecoration>())
                {
                    processComCall(inst);
                    return;
                }
            }
        }
    }

    void processInterfaceType(IRInterfaceType* interfaceType)
    {
        if (!interfaceType->findDecoration<IRComInterfaceDecoration>())
            return;
        IRBuilder builder(&sharedBuilderStorage);
        for (UInt i = 0; i < interfaceType->getOperandCount(); i++)
        {
            auto entry = as<IRInterfaceRequirementEntry>(interfaceType->getOperand(i));
            if (!entry)
                continue;
            if (auto funcType = as<IRFuncType>(entry->getRequirementVal()))
            {
                builder.setInsertBefore(funcType);
                entry->setRequirementVal(marshal.getNativeFuncType(builder, funcType));
            }
        }
    }

    void processModule()
    {
        sharedBuilderStorage.init(module);

        // Deduplicate equivalent types.
        sharedBuilderStorage.deduplicateAndRebuildGlobalNumberingMap();   

        // Translate all Calls to interface methods.
        processInstsOfType<IRCall>(kIROp_Call, [this](IRCall* inst) { processCall(inst); });

        // Update functypes of com callees.
        for (auto callee : comCallees)
        {
            IRBuilder builder(&sharedBuilderStorage);
            builder.setInsertBefore(callee);
            auto nativeType = marshal.getNativeFuncType(builder, as<IRFuncType>(callee->getDataType()));
            callee->setFullType(nativeType);
        }

        // Update func types of COM interfaces.
        processInstsOfType<IRInterfaceType>(kIROp_InterfaceType, [this](IRInterfaceType* inst) { processInterfaceType(inst); });

    }
};

void lowerComMethods(IRModule* module, DiagnosticSink* sink)
{
    ComMethodLoweringContext context(module);
    context.diagnosticSink = sink;
    context.marshal.diagnosticSink = sink;

    return context.processModule();
}
}
