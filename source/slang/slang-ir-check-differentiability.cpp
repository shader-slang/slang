#include "slang-ir-check-differentiability.h"

#include "slang-ir-diff-jvp.h"
#include "slang-ir-inst-pass-base.h"

namespace Slang
{

struct CheckDifferentiabilityPassContext : public InstPassBase
{
public:
    DiagnosticSink* sink;
    AutoDiffSharedContext sharedContext;

    HashSet<IRInst*> differentiableFunctions;

    CheckDifferentiabilityPassContext(IRModule* inModule, DiagnosticSink* inSink)
        : InstPassBase(inModule), sink(inSink), sharedContext(inModule->getModuleInst())
    {}

    IRInst* getSpecializedVal(IRInst* inst)
    {
        int loopLimit = 1024;
        while (inst && inst->getOp() == kIROp_Specialize)
        {
            inst = as<IRSpecialize>(inst)->getBase();
            loopLimit--;
            if (loopLimit == 0)
                return inst;
        }
        return inst;
    }

    IRInst* getLeafFunc(IRInst* func)
    {
        func = getSpecializedVal(func);
        if (!func)
            return nullptr;
        if (auto genericFunc = as<IRGeneric>(func))
            return findInnerMostGenericReturnVal(genericFunc);
        return func;
    }

    bool _isFuncMarkedForAutoDiff(IRInst* func)
    {
        func = getLeafFunc(func);
        if (!func)
            return false;
        for (auto decorations : func->getDecorations())
        {
            switch (decorations->getOp())
            {
            case kIROp_ForwardDifferentiableDecoration:
            case kIROp_BackwardDifferentiableDecoration:
                return true;
            }
        }
        return false;
    }


    bool _isDifferentiableFuncImpl(IRInst* func)
    {
        func = getLeafFunc(func);
        if (!func)
            return false;

        for (auto decorations : func->getDecorations())
        {
            switch (decorations->getOp())
            {
            case kIROp_ForwardDerivativeDecoration:
            case kIROp_ForwardDifferentiableDecoration:
            case kIROp_BackwardDerivativeDecoration:
            case kIROp_BackwardDifferentiableDecoration:
                return true;
            }
        }
        return false;
    }

    bool isDifferentiableFunc(IRInst* func)
    {
        switch (func->getOp())
        {
        case kIROp_ForwardDifferentiate:
        case kIROp_BackwardDifferentiate:
            return true;
        default:
            break;
        }

        func = getSpecializedVal(func);
        if (!func)
            return false;

        if (differentiableFunctions.Contains(func))
            return true;

        for (; func; func = func->parent)
        {
            if (as<IRGeneric>(func))
            {
                return differentiableFunctions.Contains(func);
            }
        }
        return false;
    }

    bool isBackwardDifferentiableFunc(IRInst* func)
    {
        for (auto decorations : func->getDecorations())
        {
            switch (decorations->getOp())
            {
            case kIROp_BackwardDerivativeDecoration:
            case kIROp_BackwardDifferentiableDecoration:
                return true;
            }
        }
        return false;
    }

    bool isDifferentiableType(DifferentiableTypeConformanceContext& context, IRInst* typeInst)
    {
        HashSet<IRInst*> processedSet;
        while (auto ptrType = as<IRPtrTypeBase>(typeInst))
        {
            typeInst = ptrType->getValueType();
            if (!processedSet.Add(typeInst))
                return false;
        }
        if (!typeInst)
            return false;
        switch (typeInst->getOp())
        {
        case kIROp_FloatType:
        case kIROp_DifferentialPairType:
            return true;
        default:
            break;
        }
        if (context.lookUpConformanceForType(typeInst))
            return true;
        // Look for equivalent types.
        for (auto type : context.differentiableWitnessDictionary)
        {
            if (isTypeEqual(type.Key, (IRType*)typeInst))
            {
                context.differentiableWitnessDictionary[(IRType*)typeInst] = type.Value;
                return true;
            }
        }
        return false;
    }

    int getParamIndexInBlock(IRParam* paramInst)
    {
        auto block = as<IRBlock>(paramInst->getParent());
        if (!block)
            return -1;
        int paramIndex = 0;
        for (auto param : block->getParams())
        {
            if (param == paramInst)
                return paramIndex;
            paramIndex++;
        }
        return -1;
    }

    bool isInstInFunc(IRInst* inst, IRInst* func)
    {
        while (inst)
        {
            if (inst == func)
                return true;
            inst = inst->parent;
        }
        return false;
    }
    void processFunc(IRGlobalValueWithCode* funcInst)
    {
        if (!_isFuncMarkedForAutoDiff(funcInst))
            return;
        if (!funcInst->getFirstBlock())
            return;

        DifferentiableTypeConformanceContext diffTypeContext(&sharedContext);
        diffTypeContext.setFunc(funcInst);

        HashSet<IRInst*> produceDiffSet;
        HashSet<IRInst*> expectDiffSet;
        int differentiableInputs = 0;
        int differentiableOutputs = 0;
        for (auto param : funcInst->getFirstBlock()->getParams())
        {
            if (isDifferentiableType(diffTypeContext, param->getFullType()))
            {
                if (as<IROutTypeBase>(param->getFullType()))
                    differentiableOutputs++;
                if (!as<IROutType>(param->getFullType()))
                    differentiableInputs++;
                produceDiffSet.Add(param);
            }
        }
        if (auto funcType = as<IRFuncType>(funcInst->getDataType()))
        {
            if (isDifferentiableType(diffTypeContext, funcType->getResultType()))
                differentiableOutputs++;
        }

        if (differentiableOutputs == 0)
            sink->diagnose(funcInst, Diagnostics::differentiableFuncMustHaveOutput);
        if (differentiableInputs == 0)
            sink->diagnose(funcInst, Diagnostics::differentiableFuncMustHaveInput);

        auto isInstProducingDiff = [&](IRInst* inst) -> bool
        {
            switch (inst->getOp())
            {
            case kIROp_FloatLit:
                return true;
            case kIROp_Call:
                return inst->findDecoration<IRNonDifferentiableCallDecoration>() || isDifferentiableFunc(as<IRCall>(inst)->getCallee());
            case kIROp_Load:
                // We don't have more knowledge on whether diff is available at the destination address.
                // Just assume it is producing diff.
                //TODO: propagate the info if this is a load of a temporary variable intended to receive result from an `out` parameter.
                return isDifferentiableType(diffTypeContext, inst->getDataType());
            default:
                // default case is to assume the inst produces a diff value if any
                // of its operands produces a diff value.
                for (UInt i = 0; i < inst->getOperandCount(); i++)
                {
                    if (produceDiffSet.Contains(inst->getOperand(i)))
                    {
                        return true;
                    }
                }
                return false;
            }
        };

        List<IRInst*> expectDiffInstWorkList;
        OrderedHashSet<IRInst*> expectDiffInstWorkListSet;
        auto addToExpectDiffWorkList = [&](IRInst* inst)
        {
            if (isInstInFunc(inst, funcInst))
            {
                if (expectDiffInstWorkListSet.Add(inst))
                    expectDiffInstWorkList.add(inst);
            }
        };
        // Run data flow analysis and generate `produceDiffSet` and an intial `expectDiffSet`.
        Index lastProduceDiffCount = 0;
        do
        {
            lastProduceDiffCount = produceDiffSet.Count();
            for (auto block : funcInst->getBlocks())
            {
                if (block != funcInst->getFirstBlock())
                {
                    UInt paramIndex = 0;
                    for (auto param : block->getParams())
                    {
                        for (auto p : block->getPredecessors())
                        {
                            // A Phi Node is producing diff if any of its candidate values are producing diff.
                            if (auto branch = as<IRUnconditionalBranch>(p->getTerminator()))
                            {
                                if (branch->getArgCount() > paramIndex)
                                {
                                    auto arg = branch->getArg(paramIndex);
                                    if (produceDiffSet.Contains(arg))
                                    {
                                        produceDiffSet.Add(param);
                                        break;
                                    }
                                }
                            }
                        }
                        paramIndex++;
                    }
                }
                for (auto inst : block->getChildren())
                {
                    if (isInstProducingDiff(inst))
                        produceDiffSet.Add(inst);
                    switch (inst->getOp())
                    {
                    case kIROp_Call:
                        if (isDifferentiableFunc(as<IRCall>(inst)->getCallee()))
                        {
                            addToExpectDiffWorkList(inst);
                        }
                        break;
                    case kIROp_Store:
                    {
                        auto storeInst = as<IRStore>(inst);
                        if (isDifferentiableType(diffTypeContext, as<IRStore>(inst)->getPtr()->getDataType()))
                        {
                            addToExpectDiffWorkList(storeInst->getVal());
                        }
                    }
                    break;
                    case kIROp_Return:
                        if (auto returnVal = as<IRReturn>(inst)->getVal())
                        {
                            if (isDifferentiableType(diffTypeContext, returnVal->getDataType()))
                            {
                                addToExpectDiffWorkList(inst);
                            }
                        }
                        break;
                    default:
                        break;
                    }
                }
            }
        } while (produceDiffSet.Count() != lastProduceDiffCount);

        // Reverse propagate `expectDiffSet`.
        for (int i = 0; i < expectDiffInstWorkList.getCount(); i++)
        {
            auto inst = expectDiffInstWorkList[i];
            // Is inst in produceDiffSet?
            if (!produceDiffSet.Contains(inst))
            {
                if (auto call = as<IRCall>(inst))
                {
                    sink->diagnose(inst, Diagnostics::lossOfDerivativeDueToCallOfNonDifferentiableFunction, getLeafFunc(call->getCallee()));
                }
            }
            switch (inst->getOp())
            {
            case kIROp_Param:
            {
                auto block = as<IRBlock>(inst->getParent());
                if (block != funcInst->getFirstBlock())
                {
                    auto paramIndex = getParamIndexInBlock(as<IRParam>(inst));
                    if (paramIndex != -1)
                    {
                        for (auto p : block->getPredecessors())
                        {
                            // A Phi Node is producing diff if any of its candidate values are producing diff.
                            if (auto branch = as<IRUnconditionalBranch>(p->getTerminator()))
                            {
                                if (branch->getArgCount() > paramIndex)
                                {
                                    auto arg = branch->getArg(paramIndex);
                                    addToExpectDiffWorkList(arg);
                                }
                            }
                        }
                    }
                }
                break;
            }
            default:
                // Default behavior is to request all differentiable operands to provide differential.
                for (UInt opIndex = 0; opIndex < inst->getOperandCount(); opIndex++)
                {
                    auto operand = inst->getOperand(opIndex);
                    if (isDifferentiableType(diffTypeContext, operand->getFullType()))
                    {
                        addToExpectDiffWorkList(operand);
                    }
                }
            }
        }
    }

    void processModule()
    {
        // Collect set of differentiable functions.
        HashSet<UnownedStringSlice> differentiableSymbolNames;
        for (auto inst : module->getGlobalInsts())
        {
            if (_isDifferentiableFuncImpl(inst))
            {
                if (auto linkageDecor = inst->findDecoration<IRLinkageDecoration>())
                    differentiableSymbolNames.Add(linkageDecor->getMangledName());
                differentiableFunctions.Add(inst);
            }
        }
        for (auto inst : module->getGlobalInsts())
        {
            if (auto linkageDecor = inst->findDecoration<IRLinkageDecoration>())
            {
                if (differentiableSymbolNames.Contains(linkageDecor->getMangledName()))
                    differentiableFunctions.Add(inst);
            }
        }

        if (!sharedContext.isInterfaceAvailable)
            return;

        for (auto inst : module->getGlobalInsts())
        {
            if (auto genericInst = as<IRGeneric>(inst))
            {
                if (auto innerFunc = as<IRGlobalValueWithCode>(findGenericReturnVal(genericInst)))
                    processFunc(innerFunc);
            }
            else if (auto funcInst = as<IRGlobalValueWithCode>(inst))
            {
                processFunc(funcInst);
            }
        }
    }
};

void checkAutoDiffUsages(IRModule* module, DiagnosticSink* sink)
{
    CheckDifferentiabilityPassContext context(module, sink);
    context.processModule();
}

} // namespace Slang
