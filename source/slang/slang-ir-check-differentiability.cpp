#include "slang-ir-check-differentiability.h"

#include "slang-ir-autodiff.h"
#include "slang-ir-inst-pass-base.h"
#include "slang-ir-sccp.h"

namespace Slang
{

struct CheckDifferentiabilityPassContext : public InstPassBase
{
public:
    DiagnosticSink* sink;
    AutoDiffSharedContext sharedContext;

    enum DiffCheckingLevel
    {
        None = 0,
        Forward = 1,
        Backward = 2
    };

    Dictionary<IRInst*, DiffCheckingLevel> differentiableFunctions;

    CheckDifferentiabilityPassContext(IRModule* inModule, DiagnosticSink* inSink)
        : InstPassBase(inModule), sink(inSink), sharedContext(nullptr, inModule->getModuleInst())
    {
    }

    DiffCheckingLevel getCheckingLevelRequirementForFunc(IRGlobalValueWithCode* code)
    {
        DiffCheckingLevel level = DiffCheckingLevel::None;

        auto addToLevel = [&](IRTranslateBase* inst)
        {
            switch (inst->getOp())
            {
            case kIROp_ForwardDifferentiate:
                level = std::max(level, DiffCheckingLevel::Forward);
                break;
            case kIROp_BackwardDifferentiate:
                level = std::max(level, DiffCheckingLevel::Backward);
                break;
            case kIROp_BackwardDifferentiatePrimal:
                level = std::max(level, DiffCheckingLevel::Backward);
                break;
            case kIROp_BackwardDifferentiatePropagate:
                level = std::max(level, DiffCheckingLevel::Backward);
                break;
            default:
                break;
            }
        };

        if (as<IRModuleInst>(code->getParent()))
        {
            traverseUsers(
                code,
                [&](IRInst* user)
                {
                    if (auto translateInst = as<IRTranslateBase>(user))
                        addToLevel(translateInst);
                });
        }
        else if (auto block = as<IRBlock>(code->getParent()))
        {
            auto generic = as<IRGeneric>(block->getParent());
            if (generic)
            {
                traverseUsers<IRSpecialize>(
                    generic,
                    [&](IRSpecialize* specialize)
                    {
                        traverseUsers(
                            specialize,
                            [&](IRInst* user)
                            {
                                if (auto translateInst = as<IRTranslateBase>(user))
                                    addToLevel(translateInst);
                            });
                    });
            }
        }

        return level;
    }

    bool shouldTreatCallAsDifferentiable(IRInst* callInst)
    {
        SLANG_ASSERT(as<IRCall>(callInst));

        return (
            callInst->findDecoration<IRTreatCallAsDifferentiableDecoration>() ||
            callInst->findDecoration<IRDifferentiableCallDecoration>());
    }

    bool isConstantVal(IRInst* inst)
    {
        switch (inst->getOp())
        {
        case kIROp_IntLit:
        case kIROp_FloatLit:
            return true;
        case kIROp_MakeValuePack:
            {
                for (UInt i = 0; i < inst->getOperandCount(); i++)
                {
                    if (!isConstantVal(inst->getOperand(i)))
                        return false;
                }
                return true;
            }
        }

        if (isEvaluableOpCode(inst->getOp()))
        {
            for (UInt i = 0; i < inst->getOperandCount(); i++)
            {
                auto operand = inst->getOperand(i);
                if (!isConstantVal(operand))
                    return false;
            }
            return true;
        }

        return false;
    }

    // If a function call takes all literals as arguments, it will implies that this function
    // will not be expected to any gradients, in this case, this call should be treated as
    // no_diff even there is no 'no_diff' decorated on it explicitly. In the actual check, we
    // only need to check the argument corresponding to the differentiable parameters, because
    // non-differentiable parameter are not expected to produce any gradients anyway.
    bool shouldCallImpliesNoDiff(
        DifferentiableTypeConformanceContext& diffTypeContext,
        IRCall* callInst)
    {
        if (shouldTreatCallAsDifferentiable(callInst))
        {
            return true;
        }

        auto calleeFuncType = as<IRFuncType>(callInst->getCallee()->getFullType());
        if (!calleeFuncType)
            return false;

        SLANG_RELEASE_ASSERT(calleeFuncType->getParamCount() == callInst->getArgCount());

        bool doesImplyNoDiff = true;
        UInt paramIndex = 0;
        for (auto paramType : calleeFuncType->getParamTypes())
        {
            auto [paramDirectionInfo, paramBaseType] = splitParameterDirectionAndType(paramType);
            if (diffTypeContext.isDifferentiableType(paramBaseType))
            {
                auto arg = callInst->getArg(paramIndex);
                if (!isConstantVal(arg))
                {
                    doesImplyNoDiff = false;
                }
            }
            paramIndex++;
        }

        if (doesImplyNoDiff)
        {
            IRBuilder irBuilder(callInst->getModule());
            irBuilder.addDecoration(callInst, kIROp_TreatCallAsDifferentiableDecoration);
        }
        return doesImplyNoDiff;
    }

    bool isDifferentiableFunc(
        DifferentiableTypeConformanceContext& ctx,
        IRInst* func,
        DiffCheckingLevel level)
    {
        if (ctx.tryGetAssociationOfKind(func, ValAssociationKind::ForwardDerivative))
        {
            if (level == DiffCheckingLevel::Forward)
                return true;
        }

        if (ctx.tryGetAssociationOfKind(func, ValAssociationKind::BackwardDerivativeApply))
        {
            return true;
        }
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

    bool canAddressHoldDerivative(
        DifferentiableTypeConformanceContext& diffTypeContext,
        IRInst* addr)
    {
        if (!addr)
            return false;

        while (addr)
        {
            switch (addr->getOp())
            {
            case kIROp_Var:
                {
                    auto valueType = as<IRPtrTypeBase>(addr->getDataType())->getValueType();
                    return diffTypeContext.isDifferentiableType(valueType);
                }
            case kIROp_Param:
                {
                    auto [passingMode, valueType] =
                        splitParameterDirectionAndType(addr->getDataType());
                    return diffTypeContext.isDifferentiableType(valueType);
                }
            case kIROp_FieldAddress:
                if (!as<IRFieldAddress>(addr)->getField() ||
                    as<IRFieldAddress>(addr)
                            ->getField()
                            ->findDecoration<IRDerivativeMemberDecoration>() == nullptr)
                    return false;
                addr = as<IRFieldAddress>(addr)->getBase();
                break;
            case kIROp_GetElementPtr:
                if (!diffTypeContext.isDifferentiableType(
                        as<IRPtrTypeBase>(as<IRGetElementPtr>(addr)->getBase()->getDataType())
                            ->getValueType()))
                    return false;
                addr = as<IRGetElementPtr>(addr)->getBase();
                break;
            default:
                return false;
            }
        }
        return false;
    }

    bool instHasNonTrivialDerivative(
        DifferentiableTypeConformanceContext& diffTypeContext,
        IRInst* inst)
    {
        switch (inst->getOp())
        {
        case kIROp_DetachDerivative:
            return false;
        case kIROp_Call:
            {
                auto call = as<IRCall>(inst);
                return isDifferentiableFunc(
                    diffTypeContext,
                    call->getCallee(),
                    CheckDifferentiabilityPassContext::DiffCheckingLevel::Forward);
            }
        default:
            return diffTypeContext.isDifferentiableType(inst->getDataType());
        }
    }

    bool checkType(IRInst* type)
    {
        type = unwrapAttributedType(type);
        if (as<IRTorchTensorType>(type))
            return false;
        else if (auto arrayType = as<IRArrayTypeBase>(type))
            return checkType(arrayType->getElementType());
        else if (auto structType = as<IRStructType>(type))
        {
            for (auto field : structType->getFields())
            {
                if (!checkType(field->getFieldType()))
                    return false;
            }
        }
        return true;
    }
    void checkForInvalidHostTypeUsage(IRGlobalValueWithCode* funcInst)
    {
        auto outerFuncInst = maybeFindOuterGeneric(funcInst);

        if (outerFuncInst->findDecoration<IRCudaHostDecoration>())
            return;
        if (outerFuncInst->findDecoration<IRTorchEntryPointDecoration>())
            return;

        bool isSynthesizeConstructor = false;

        if (auto constructor = funcInst->findDecoration<IRConstructorDecoration>())
            isSynthesizeConstructor = constructor->getSynthesizedStatus();

        // This is a kernel function, we don't allow using TorchTensor type here.
        for (auto b : funcInst->getBlocks())
        {
            for (auto inst : b->getChildren())
            {
                if (!checkType(inst->getDataType()))
                {
                    if (isSynthesizeConstructor)
                    {
                        IRBuilder irBuilder(funcInst);
                        irBuilder.addDecoration(funcInst, kIROp_CudaHostDecoration);
                        return;
                    }

                    auto loc = inst->sourceLoc;
                    if (!loc.isValid())
                        loc = funcInst->sourceLoc;
                    sink->diagnose(loc, Diagnostics::invalidUseOfTorchTensorTypeInDeviceFunc);
                    return;
                }
            }
        }
    }

    void processFunc(IRGlobalValueWithCode* funcInst)
    {
        checkForInvalidHostTypeUsage(funcInst);

        auto requiredDiffLevel = getCheckingLevelRequirementForFunc(funcInst);
        if (requiredDiffLevel == DiffCheckingLevel::None)
            return;

        if (!funcInst->getFirstBlock())
            return;

        DifferentiableTypeConformanceContext diffTypeContext(&sharedContext);

        // We compute and track three different set of insts to complete our
        // data flow analysis.
        // `produceDiffSet` represents a set of insts that can provide a diff. This is conservative
        // on the positive side: a float literal is considered to be able to provide a diff.
        // `carryNonTrivialDiffSet` represents a set of insts that may carry a non-zero diff. This
        // is conservative on the negative side: if the inst does not provide a diff, or if we can
        // prove the diff is zero, we exclude the inst from the set. This makes
        // `carryNonTrivialDiffSet` a strict subset of `produceDiffSet`. `expectDiffSet` is a set of
        // insts that expects their operands to produce a diff. It is an error if they don't.
        InstHashSet produceDiffSet(funcInst->getModule());
        InstHashSet expectDiffSet(funcInst->getModule());
        InstHashSet carryNonTrivialDiffSet(funcInst->getModule());

        bool isDifferentiableReturnType = false;
        for (auto param : funcInst->getFirstBlock()->getParams())
        {
            auto [_, paramBaseType] = splitParameterDirectionAndType(param->getDataType());
            if (diffTypeContext.isDifferentiableType(paramBaseType))
            {
                produceDiffSet.add(param);
                carryNonTrivialDiffSet.add(param);
            }
        }
        if (auto funcType = as<IRFuncType>(funcInst->getDataType()))
        {
            if (diffTypeContext.isDifferentiableType(funcType->getResultType()))
            {
                isDifferentiableReturnType = true;
            }
        }

        auto isInstProducingDiff = [&](IRInst* inst) -> bool
        {
            switch (inst->getOp())
            {
            case kIROp_FloatLit:
                return true;
            case kIROp_Call:
                return shouldTreatCallAsDifferentiable(inst) ||
                       isDifferentiableFunc(
                           diffTypeContext,
                           as<IRCall>(inst)->getCallee(),
                           requiredDiffLevel) &&
                           diffTypeContext.isDifferentiableType(inst->getFullType());
            case kIROp_Load:
                // We don't have more knowledge on whether diff is available at the destination
                // address. Just assume it is producing diff if the dest address can hold a
                // derivative.
                // TODO: propagate the info if this is a load of a temporary variable intended
                // to receive result from an `out` parameter.
                return canAddressHoldDerivative(diffTypeContext, as<IRLoad>(inst)->getPtr());
            default:
                // default case is to assume the inst produces a diff value if any
                // of its operands produces a diff value.
                if (!diffTypeContext.isDifferentiableType(inst->getFullType()))
                    return false;
                for (UInt i = 0; i < inst->getOperandCount(); i++)
                {
                    if (produceDiffSet.contains(inst->getOperand(i)))
                    {
                        return true;
                    }
                }
                return false;
            }
        };

        auto isInstCarryingOverDiff = [&](IRInst* inst) -> bool
        {
            switch (inst->getOp())
            {
            case kIROp_DetachDerivative:
                return false;
            case kIROp_Call:
                if (shouldTreatCallAsDifferentiable(inst))
                    return false;
                return isDifferentiableFunc(
                           diffTypeContext,
                           as<IRCall>(inst)->getCallee(),
                           requiredDiffLevel) &&
                       diffTypeContext.isDifferentiableType(inst->getFullType());
            case kIROp_Load:
                // We don't have more knowledge on whether diff is available at the destination
                // address. Just assume it is producing diff if the dest address can hold a
                // derivative.
                // TODO: propagate the info if this is a load of a temporary variable intended
                // to receive result from an `out` parameter.
                return canAddressHoldDerivative(diffTypeContext, as<IRLoad>(inst)->getPtr());
            default:
                // default case is to assume the inst produces a diff value if any
                // of its operands produces a diff value.
                if (!diffTypeContext.isDifferentiableType(inst->getFullType()))
                    return false;
                for (UInt i = 0; i < inst->getOperandCount(); i++)
                {
                    if (carryNonTrivialDiffSet.contains(inst->getOperand(i)))
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
                if (expectDiffInstWorkListSet.add(inst))
                {
                    expectDiffInstWorkList.add(inst);
                }
            }
        };

        // Run data flow analysis and generate `produceDiffSet` and an intial `expectDiffSet`.
        Index lastProduceDiffCount = 0;
        do
        {
            lastProduceDiffCount = produceDiffSet.getCount();
            for (auto block : funcInst->getBlocks())
            {
                if (block != funcInst->getFirstBlock())
                {
                    UInt paramIndex = 0;
                    for (auto param : block->getParams())
                    {
                        for (auto p : block->getPredecessors())
                        {
                            // A Phi Node is producing diff if any of its candidate values are
                            // producing diff.
                            if (auto branch = as<IRUnconditionalBranch>(p->getTerminator()))
                            {
                                if (branch->getArgCount() > paramIndex)
                                {
                                    auto arg = branch->getArg(paramIndex);
                                    if (produceDiffSet.contains(arg))
                                        produceDiffSet.add(param);
                                    if (carryNonTrivialDiffSet.contains(arg))
                                        carryNonTrivialDiffSet.add(param);
                                }
                            }
                        }
                        paramIndex++;
                    }
                }
                for (auto inst : block->getChildren())
                {
                    if (isInstProducingDiff(inst))
                        produceDiffSet.add(inst);
                    if (isInstCarryingOverDiff(inst))
                        carryNonTrivialDiffSet.add(inst);
                    switch (inst->getOp())
                    {
                    case kIROp_Call:
                        if (isDifferentiableFunc(
                                diffTypeContext,
                                as<IRCall>(inst)->getCallee(),
                                requiredDiffLevel))
                        {
                            addToExpectDiffWorkList(inst);
                        }
                        break;
                    case kIROp_Store:
                        {
                            auto storeInst = as<IRStore>(inst);
                            if (canAddressHoldDerivative(diffTypeContext, storeInst->getPtr()) &&
                                diffTypeContext.isDifferentiableType(
                                    as<IRStore>(inst)->getPtr()->getDataType()))
                            {
                                addToExpectDiffWorkList(storeInst->getVal());
                            }
                        }
                        break;
                    case kIROp_Return:
                        if (auto returnVal = as<IRReturn>(inst)->getVal())
                        {
                            if (isDifferentiableReturnType &&
                                diffTypeContext.isDifferentiableType(returnVal->getDataType()))
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
        } while (produceDiffSet.getCount() != lastProduceDiffCount);

        // Reverse propagate `expectDiffSet`.
        for (int i = 0; i < expectDiffInstWorkList.getCount(); i++)
        {
            auto inst = expectDiffInstWorkList[i];
            // Is inst in produceDiffSet?
            if (!produceDiffSet.contains(inst))
            {
                if (auto call = as<IRCall>(inst))
                {
                    const auto callee = call->getCallee();
                    // If inst's type is differentiable, and it is in expectDiffInstWorkList,
                    // then some user is expecting the result of the call to produce a derivative.
                    // In this case we need to issue a diagnostic.
                    if (diffTypeContext.isDifferentiableType(inst->getFullType()) &&
                        !isDifferentiableFunc(diffTypeContext, callee, requiredDiffLevel))
                    {
                        // No need to fail here if the function is no_diff in
                        // both inputs and all outputs, this is equivalent of
                        // inserting no_diff on this inst.
                        if (!isNeverDiffFuncType(cast<IRFuncType>(callee->getDataType())) &&
                            !shouldCallImpliesNoDiff(diffTypeContext, call))
                        {
                            sink->diagnose(
                                inst,
                                Diagnostics::lossOfDerivativeDueToCallOfNonDifferentiableFunction,
                                getResolvedInstForDecorations(call->getCallee()),
                                requiredDiffLevel == DiffCheckingLevel::Forward ? "forward"
                                                                                : "backward");
                        }
                    }
                }
            }
            switch (inst->getOp())
            {
            case kIROp_Param:
                {
                    auto block = as<IRBlock>(inst->getParent());
                    if (block != funcInst->getFirstBlock())
                    {
                        auto paramIndex = getParamIndexInBlock(
                            as<IRParam, IRDynamicCastBehavior::NoUnwrap>(inst));
                        if (paramIndex != -1)
                        {
                            for (auto p : block->getPredecessors())
                            {
                                // A Phi Node is producing diff if any of its candidate values are
                                // producing diff.
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
            case kIROp_Call:
                {
                    auto callInst = as<IRCall>(inst);
                    if (callInst->findDecoration<IRTreatCallAsDifferentiableDecoration>())
                        continue;
                    auto calleeFuncType = as<IRFuncType>(callInst->getCallee()->getFullType());
                    if (!calleeFuncType)
                        continue;
                    if (calleeFuncType->getParamCount() != callInst->getArgCount())
                        continue;
                    for (UInt a = 0; a < callInst->getArgCount(); a++)
                    {
                        auto arg = callInst->getArg(a);
                        auto paramType = calleeFuncType->getParamType(a);
                        auto [_, paramBaseType] = splitParameterDirectionAndType(paramType);
                        if (!diffTypeContext.isDifferentiableType(paramBaseType))
                            continue;
                        addToExpectDiffWorkList(arg);
                    }
                    break;
                }
            default:
                // Default behavior is to request all differentiable operands to provide
                // differential.
                for (UInt opIndex = 0; opIndex < inst->getOperandCount(); opIndex++)
                {
                    auto operand = inst->getOperand(opIndex);
                    if (diffTypeContext.isDifferentiableType(operand->getFullType()))
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
            if (loop->findDecoration<IRLoopMaxItersDecoration>() ||
                loop->findDecoration<IRForceUnrollDecoration>())
            {
                // We are good.
            }
            else
            {
                sink->diagnose(loop->sourceLoc, Diagnostics::loopInDiffFuncRequireUnrollOrMaxIters);
            }
        }

        // Make sure all stores of differentiable values are into addresses that can hold
        // derivatives. If we are assigning a value to a non-differentiable location, we need to
        // make sure that value doesn't carray a non-zero diff.
        for (auto block : funcInst->getBlocks())
        {
            for (auto inst : block->getChildren())
            {
                if (auto storeInst = as<IRStore>(inst))
                {
                    if (carryNonTrivialDiffSet.contains(storeInst->getVal()) &&
                        !canAddressHoldDerivative(diffTypeContext, storeInst->getPtr()))
                    {
                        sink->diagnose(
                            storeInst->sourceLoc,
                            Diagnostics::lossOfDerivativeAssigningToNonDifferentiableLocation);
                    }
                }
                else if (auto callInst = as<IRCall>(inst))
                {
                    if (!isDifferentiableFunc(
                            diffTypeContext,
                            callInst->getCallee(),
                            DiffCheckingLevel::Forward))
                        continue;
                    auto calleeFuncType = as<IRFuncType>(callInst->getCallee()->getFullType());
                    if (!calleeFuncType)
                        continue;
                    if (calleeFuncType->getParamCount() != callInst->getArgCount())
                        continue;
                    for (UInt a = 0; a < callInst->getArgCount(); a++)
                    {
                        auto arg = callInst->getArg(a);
                        auto paramType = calleeFuncType->getParamType(a);

                        auto [paramDirectionInfo, paramBaseType] =
                            splitParameterDirectionAndType(paramType);
                        if (!diffTypeContext.isDifferentiableType(paramBaseType))
                            continue;

                        if (paramDirectionInfo.kind == ParameterDirectionInfo::Kind::Out ||
                            paramDirectionInfo.kind == ParameterDirectionInfo::Kind::BorrowInOut ||
                            paramDirectionInfo.kind == ParameterDirectionInfo::Kind::Ref)
                        {
                            if (!canAddressHoldDerivative(diffTypeContext, arg))
                            {
                                sink->diagnose(
                                    arg->sourceLoc,
                                    Diagnostics::
                                        lossOfDerivativeUsingNonDifferentiableLocationAsOutArg);
                            }
                        }
                    }
                }
            }
        }
    }

    void processModule()
    {
        // Collect set of differentiable functions.
        HashSet<UnownedStringSlice> fwdDifferentiableSymbolNames, bwdDifferentiableSymbolNames;
        DifferentiableTypeConformanceContext ctx(&sharedContext);

        for (auto inst : module->getGlobalInsts())
        {
            if (isDifferentiableFunc(ctx, inst, DiffCheckingLevel::Backward))
            {
                if (auto linkageDecor = inst->findDecoration<IRLinkageDecoration>())
                    bwdDifferentiableSymbolNames.add(linkageDecor->getMangledName());
                differentiableFunctions.add(inst, DiffCheckingLevel::Backward);
            }
            else if (isDifferentiableFunc(ctx, inst, DiffCheckingLevel::Forward))
            {
                if (auto linkageDecor = inst->findDecoration<IRLinkageDecoration>())
                    fwdDifferentiableSymbolNames.add(linkageDecor->getMangledName());
                differentiableFunctions.add(inst, DiffCheckingLevel::Forward);
            }
        }
        for (auto inst : module->getGlobalInsts())
        {
            if (auto linkageDecor = inst->findDecoration<IRLinkageDecoration>())
            {
                if (bwdDifferentiableSymbolNames.contains(linkageDecor->getMangledName()))
                    differentiableFunctions[inst] = DiffCheckingLevel::Backward;
                else if (fwdDifferentiableSymbolNames.contains(linkageDecor->getMangledName()))
                    differentiableFunctions.addIfNotExists(inst, DiffCheckingLevel::Forward);
            }
        }

        if (!sharedContext.isInterfaceAvailable && !sharedContext.isPtrInterfaceAvailable)
            return;

        for (auto inst : module->getGlobalInsts())
        {
            if (auto genericInst = as<IRGeneric>(inst))
            {
                if (auto innerFunc =
                        as<IRGlobalValueWithCode>(findInnerMostGenericReturnVal(genericInst)))
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
