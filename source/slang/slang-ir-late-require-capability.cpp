// slang-ir-late-require-capability.cpp

#include "slang-ir-late-require-capability.h"

#include "slang-ir-call-graph.h"
#include "slang-ir-insts.h"
#include "slang-ir.h"
#include "slang-profile.h"
#include "slang-target.h"
#include "slang.h"

namespace Slang
{

struct CheckLateRequireCapabilityInstsContext
{
    IRModule* m_module;
    DiagnosticSink* m_sink;
    SlangResult m_status = SLANG_OK;

    void checkFunc(IRFunc* func)
    {
        // scan the function for IRLateRequireCapability instructions
        for (auto block : func->getBlocks())
        {
            for (auto inst : block->getOrdinaryInsts())
            {
                if (auto lateRequireCap = as<IRLateRequireCapability>(inst))
                {
                    for (UInt i = 0; i < lateRequireCap->getOperandCount(); ++i)
                    {
                        IRConstant* capConstant = as<IRConstant>(inst->getOperand(i));
                        if (!capConstant)
                        {
                            m_sink->diagnose(inst, Diagnostics::expectedAStringLiteral);
                            m_status = SLANG_FAIL;
                        }
                        else
                        {
                            UnownedStringSlice capNameStr = capConstant->getStringSlice();
                            CapabilityName capName = findCapabilityName(capNameStr);
                            if (capName == CapabilityName::Invalid)
                            {
                                m_sink->diagnose(inst, Diagnostics::unknownCapability, capNameStr);
                                m_status = SLANG_FAIL;
                            }
                        }
                    }
                }
            }
        }
    }

    void checkModule()
    {
        for (auto inst = m_module->getModuleInst()->getFirstChild(); inst;
             inst = inst->getNextInst())
        {
            auto func = as<IRFunc>(inst);
            if (!func)
                continue;

            checkFunc(func);
        }
    }
};

struct ProcessLateRequireCapabilityInstsContext
{
    IRModule* const m_module;
    CapabilitySet m_targetCaps;
    const CodeGenTarget m_target;
    CompilerOptionSet& m_optionSet;
    DiagnosticSink* const m_sink;
    SlangResult m_status = SLANG_OK;

    Dictionary<IRInst*, HashSet<IRFunc*>> m_mapInstToReferencingEntryPoints;

    ProcessLateRequireCapabilityInstsContext(
        IRModule* module,
        const CapabilitySet& targetCaps,
        CodeGenTarget target,
        CompilerOptionSet& optionSet,
        DiagnosticSink* sink)
        : m_module(module)
        , m_targetCaps(targetCaps)
        , m_target(target)
        , m_optionSet(optionSet)
        , m_sink(sink)
    {
    }

    void checkCapability(
        IRFunc* entry,
        IRLateRequireCapability* inst,
        Profile profile,
        List<CapabilityName> capNames)
    {
        CapabilitySet targetCaps = m_targetCaps;
        auto stageCapabilitySet = profile.getCapabilityName();
        targetCaps.join(stageCapabilitySet);
        CapabilitySet required(capNames);

        // check that we have the required caps for this stage
        if (targetCaps.atLeastOneSetImpliedInOther(required) ==
            CapabilitySet::ImpliesReturnFlags::Implied)
            return;

        required.join(stageCapabilitySet);

        // figure out the missing delta
        CapabilityAtomSet addedAtoms{};

        if (auto stageCapSet = targetCaps.getAtomSets())
        {
            if (auto requiredSet = required.getAtomSets())
            {
                CapabilityAtomSet::calcSubtract(addedAtoms, (*requiredSet), (*stageCapSet));
            }
        }

#if 0
        StringBuilder sb;
        printDiagnosticArg(sb, stageCapabilitySet);
        fprintf(stderr, "stageCapabilitySet: %s\n", sb.toString().getBuffer());

        sb.clear();
        printDiagnosticArg(sb, required);
        fprintf(stderr, "required:           %s\n", sb.toString().getBuffer());

        sb.clear();
        printDiagnosticArg(sb, addedAtoms);
        fprintf(stderr, "difference:         %s\n", sb.toString().getBuffer());
#endif

        maybeDiagnoseWarningOrError(
            m_sink,
            m_optionSet,
            DiagnosticCategory::Capability,
            getDiagnosticPos(entry),
            Diagnostics::profileImplicitlyUpgraded,
            Diagnostics::profileImplicitlyUpgradedRestrictive,
            entry,
            m_optionSet.getProfile().getName(),
            addedAtoms.getElements<CapabilityAtom>());

        m_sink->diagnose(inst->sourceLoc, Diagnostics::seeCallOfFunc, "__requireCapability()");

        // we'll fail only if restrictive capability check was requested
        if (m_optionSet.getBoolOption(CompilerOptionName::RestrictiveCapabilityCheck))
            m_status = SLANG_FAIL;
    }

    void processCapability(IRFunc* entry, Profile profile, IRLateRequireCapability* inst)
    {
        List<CapabilityName> capNames;

        for (UInt i = 0; i < inst->getOperandCount(); ++i)
        {
            IRConstant* capConstant = as<IRConstant>(inst->getOperand(i));

            // note: capName validity has already been checked by
            // CheckLateRequireCapabilityInstsContext
            UnownedStringSlice capNameStr = capConstant->getStringSlice();
            CapabilityName capName = findCapabilityName(capNameStr);
            capNames.add(capName);
        }

        checkCapability(entry, inst, profile, capNames);
    }

    void processFunc(IRFunc* func)
    {
        List<IRLateRequireCapability*> instsToRemove;

        // scan the function for IRLateRequireCapability instructions
        for (auto block : func->getBlocks())
        {
            for (auto inst : block->getOrdinaryInsts())
            {
                if (auto lateRequireCap = as<IRLateRequireCapability>(inst))
                {
                    instsToRemove.add(lateRequireCap);

                    const HashSet<IRFunc*>* entryPoints =
                        m_mapInstToReferencingEntryPoints.tryGetValue(func);

                    if (entryPoints)
                    {
                        for (auto entryPoint : *entryPoints)
                        {
                            if (auto entryPointDecor =
                                    entryPoint->findDecoration<IREntryPointDecoration>())
                            {
                                processCapability(
                                    entryPoint,
                                    entryPointDecor->getProfile(),
                                    lateRequireCap);
                            }
                        }
                    }
                }
            }
        }

        for (auto lateRequireCap : instsToRemove)
        {
            lateRequireCap->removeAndDeallocate();
        }
    }

    void processModule()
    {
        buildEntryPointReferenceGraph(m_mapInstToReferencingEntryPoints, m_module);

        for (auto inst = m_module->getModuleInst()->getFirstChild(); inst;
             inst = inst->getNextInst())
        {
            auto func = as<IRFunc>(inst);
            if (!func)
                continue;

            processFunc(func);
        }
    }
};

/// Checks that the named capabilities exist.
SlangResult checkLateRequireCapabilityArguments(IRModule* module, DiagnosticSink* sink)
{
    CheckLateRequireCapabilityInstsContext context(module, sink);

    context.checkModule();
    return context.m_status;
}

SlangResult processLateRequireCapabilityInsts(
    IRModule* module,
    CodeGenContext* codeGenContext,
    DiagnosticSink* sink)
{
    ProcessLateRequireCapabilityInstsContext context(
        module,
        codeGenContext->getTargetCaps(),
        codeGenContext->getTargetFormat(),
        codeGenContext->getTargetReq()->getOptionSet(),
        sink);

    context.processModule();
    return context.m_status;
}

} // namespace Slang
