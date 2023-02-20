#include "slang-ir-check-differentiability.h"

#include "slang-ir-autodiff.h"
#include "slang-ir-inst-pass-base.h"

namespace Slang
{

struct CheckDifferentiabilityPassContext : public InstPassBase
{
public:
    DiagnosticSink* sink;
    AutoDiffSharedContext sharedContext;

    enum DifferentiableLevel
    {
        Forward, Backward
    };
    Dictionary<IRInst*, DifferentiableLevel> differentiableFunctions;

    CheckDifferentiabilityPassContext(IRModule* inModule, DiagnosticSink* inSink)
        : InstPassBase(inModule), sink(inSink), sharedContext(inModule->getModuleInst())
    {}

    bool _isFuncMarkedForAutoDiff(IRInst* func)
    {
        func = getResolvedInstForDecorations(func);
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


    bool _isDifferentiableFuncImpl(IRInst* func, DifferentiableLevel level)
    {
        func = getResolvedInstForDecorations(func);
        if (!func)
            return false;

        for (auto decorations : func->getDecorations())
        {
            switch (decorations->getOp())
            {
            case kIROp_ForwardDerivativeDecoration:
            case kIROp_ForwardDifferentiableDecoration:
                if (level == DifferentiableLevel::Forward)
                    return true;
                break;
            case kIROp_UserDefinedBackwardDerivativeDecoration:
            case kIROp_BackwardDerivativeDecoration:
            case kIROp_BackwardDifferentiableDecoration:
                return true;
            default:
                break;
            }
        }
        return false;
    }

    bool isDifferentiableFunc(IRInst* func, DifferentiableLevel level)
    {
        if (level == DifferentiableLevel::Forward)
        {
            switch (func->getOp())
            {
            case kIROp_ForwardDifferentiate:
            case kIROp_BackwardDifferentiate:
                return true;
            default:
                break;
            }
        }

        func = getResolvedInstForDecorations(func);
        if (!func)
            return false;

        
        if (auto existingLevel = differentiableFunctions.TryGetValue(func))
            return *existingLevel >= level;

        if (func->findDecoration<IRTreatAsDifferentiableDecoration>())
            return true;

        if (auto lookupInterfaceMethod = as<IRLookupWitnessMethod>(func))
        {
            auto wit = lookupInterfaceMethod->getWitnessTable();
            if (!wit)
                return false;
            auto witType = as<IRWitnessTableTypeBase>(wit->getDataType());
            if (!witType)
                return false;
            auto interfaceType = witType->getConformanceType();
            if (!interfaceType)
                return false;
            if (interfaceType->findDecoration<IRTreatAsDifferentiableDecoration>())
                return true;
            if (sharedContext.differentiableInterfaceType && interfaceType == sharedContext.differentiableInterfaceType)
                return true;
            if (lookupInterfaceMethod->getRequirementKey()->findDecoration<IRBackwardDerivativeDecoration>())
                return true;
            if (lookupInterfaceMethod->getRequirementKey()->findDecoration<IRForwardDerivativeDecoration>())
                return level == DifferentiableLevel::Forward;
        }

        for (; func; func = func->parent)
        {
            if (as<IRGeneric>(func))
            {
                if (auto existingLevel = differentiableFunctions.TryGetValue(func))
                {
                    if (*existingLevel >= level)
                        return true;
                }
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
        int differentiableOutputs = 0;
        for (auto param : funcInst->getFirstBlock()->getParams())
        {
            if (isDifferentiableType(diffTypeContext, param->getFullType()))
            {
                if (as<IROutTypeBase>(param->getFullType()))
                    differentiableOutputs++;
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

        DifferentiableLevel requiredDiffLevel = DifferentiableLevel::Forward;
        if (isBackwardDifferentiableFunc(funcInst))
            requiredDiffLevel = DifferentiableLevel::Backward;

        auto isInstProducingDiff = [&](IRInst* inst) -> bool
        {
            switch (inst->getOp())
            {
            case kIROp_FloatLit:
                return true;
            case kIROp_Call:
                return inst->findDecoration<IRTreatAsDifferentiableDecoration>() || isDifferentiableFunc(as<IRCall>(inst)->getCallee(), requiredDiffLevel);
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
                        if (isDifferentiableFunc(as<IRCall>(inst)->getCallee(), requiredDiffLevel))
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
                    sink->diagnose(
                        inst,
                        Diagnostics::lossOfDerivativeDueToCallOfNonDifferentiableFunction,
                        getResolvedInstForDecorations(call->getCallee()),
                        requiredDiffLevel == DifferentiableLevel::Forward ? "forward" : "backward");
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
                                if (branch->getArgCount() > (UInt)paramIndex)
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

        // Make sure all loops are marked with either [MaxIters] or [ForceUnroll].
        for (auto block : funcInst->getBlocks())
        {
            auto loop = as<IRLoop>(block->getTerminator());
            if (!loop)
                continue;
            bool hasBackEdge = false;
            for (auto use = loop->getTargetBlock()->firstUse; use; use = use->nextUse)
            {
                if (use->getUser() != loop)
                {
                    hasBackEdge = true;
                    break;
                }
            }
            if (!hasBackEdge)
                continue;
            if (loop->findDecoration<IRLoopMaxItersDecoration>() || loop->findDecoration<IRForceUnrollDecoration>())
            {
                // We are good.
            }
            else
            {
                sink->diagnose(loop->sourceLoc, Diagnostics::loopInDiffFuncRequireUnrollOrMaxIters);
            }
        }
    }

    void processModule()
    {
        // Collect set of differentiable functions.
        HashSet<UnownedStringSlice> fwdDifferentiableSymbolNames, bwdDifferentiableSymbolNames;
        for (auto inst : module->getGlobalInsts())
        {
            if (_isDifferentiableFuncImpl(inst, DifferentiableLevel::Backward))
            {
                if (auto linkageDecor = inst->findDecoration<IRLinkageDecoration>())
                    bwdDifferentiableSymbolNames.Add(linkageDecor->getMangledName());
                differentiableFunctions.Add(inst, DifferentiableLevel::Backward);
            }
            else if (_isDifferentiableFuncImpl(inst, DifferentiableLevel::Forward))
            {
                if (auto linkageDecor = inst->findDecoration<IRLinkageDecoration>())
                    fwdDifferentiableSymbolNames.Add(linkageDecor->getMangledName());
                differentiableFunctions.Add(inst, DifferentiableLevel::Forward);
            }
        }
        for (auto inst : module->getGlobalInsts())
        {
            if (auto linkageDecor = inst->findDecoration<IRLinkageDecoration>())
            {
                if (bwdDifferentiableSymbolNames.Contains(linkageDecor->getMangledName()))
                    differentiableFunctions[inst] = DifferentiableLevel::Backward;
                else if (fwdDifferentiableSymbolNames.Contains(linkageDecor->getMangledName()))
                    differentiableFunctions.AddIfNotExists(inst, DifferentiableLevel::Forward);
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
