// slang-ir-cleanup-void.cpp

#include "slang-ir-cleanup-void.h"
#include "slang-ir.h"
#include "slang-ir-insts.h"

namespace Slang
{
    struct CleanUpVoidContext
    {
        IRModule* module;

        SharedIRBuilder sharedBuilderStorage;

        List<IRInst*> workList;
        HashSet<IRInst*> workListSet;

        void addToWorkList(
            IRInst* inst)
        {
            for (auto ii = inst->getParent(); ii; ii = ii->getParent())
            {
                if (as<IRGeneric>(ii))
                    return;
            }

            if (workListSet.Contains(inst))
                return;

            workList.add(inst);
            workListSet.Add(inst);
        }

        void processInst(IRInst* inst)
        {
            switch (inst->getOp())
            {
            case kIROp_Call:
                {
                    // Remove void argument.
                    auto call = as<IRCall>(inst);
                    List<IRInst*> newArgs;
                    for (UInt i = 0; i < call->getArgCount(); i++)
                    {
                        auto arg = call->getArg(i);
                        if (arg->getDataType() && arg->getDataType()->getOp() == kIROp_VoidType)
                        {
                            continue;
                        }
                        newArgs.add(arg);
                    }
                    if (newArgs.getCount() != (Index)call->getArgCount())
                    {
                        IRBuilder builder(&sharedBuilderStorage);
                        builder.setInsertBefore(call);
                        auto newCall = builder.emitCallInst(call->getFullType(), call->getCallee(), newArgs);
                        call->replaceUsesWith(newCall);
                        call->removeAndDeallocate();
                        inst = newCall;
                    }
                }
                break;
            case kIROp_Func:
                {
                    // Remove void parameter.
                    List<IRParam*> paramsToRemove;
                    auto func = as<IRFunc>(inst);
                    for (auto param : func->getParams())
                    {
                        if (param->getDataType()->getOp() == kIROp_VoidType)
                        {
                            paramsToRemove.add(param);
                        }
                    }
                    IRBuilder builder(&sharedBuilderStorage);
                    builder.setInsertBefore(func);
                    for (auto param : paramsToRemove)
                    {
                        auto voidVal = builder.getVoidValue();
                        param->replaceUsesWith(voidVal);
                        param->removeAndDeallocate();
                    }
                }
                break;
            case kIROp_FuncType:
                {
                    auto funcType = as<IRFuncType>(inst);
                    List<IRInst*> newOperands;
                    for (UInt i = 1; i < funcType->getOperandCount(); i++)
                    {
                        auto operand = funcType->getOperand(i);
                        if (operand->getOp() == kIROp_VoidType)
                        {
                            continue;
                        }
                        newOperands.add(operand);
                    }
                    if (newOperands.getCount() != (Index)funcType->getParamCount())
                    {
                        IRBuilder builder(&sharedBuilderStorage);
                        builder.setInsertBefore(funcType);
                        auto newFuncType = builder.getFuncType(newOperands.getCount(), (IRType**)newOperands.getBuffer(), funcType->getResultType());
                        if (newFuncType != funcType)
                        {
                            funcType->replaceUsesWith(newFuncType);
                            funcType->removeAndDeallocate();
                        }
                        inst = newFuncType;
                    }
                }
                break;
            case kIROp_StructType:
                {
                    // TODO: cleanup void fields.
                }
                break;
            default:
                break;
            }

            // TODO: If inst has void type, all uses of it should be replaced with void val.
            // We should do this only for a subset of opcodes known to be safe.

        }

        void processModule()
        {
            SharedIRBuilder* sharedBuilder = &sharedBuilderStorage;
            sharedBuilder->init(module);

            // Deduplicate equivalent types.
            sharedBuilder->deduplicateAndRebuildGlobalNumberingMap();

            addToWorkList(module->getModuleInst());

            while (workList.getCount() != 0)
            {
                IRInst* inst = workList.getLast();

                workList.removeLast();
                workListSet.Remove(inst);

                processInst(inst);

                for (auto child = inst->getLastChild(); child; child = child->getPrevInst())
                {
                    addToWorkList(child);
                }
            }
        }
    };
    
    void cleanUpVoidType(IRModule* module)
    {
        CleanUpVoidContext context;
        context.module = module;
        context.processModule();
    }
}
