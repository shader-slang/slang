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

    void checkCapability(IRFunc* entry, Profile profile, IRCapabilitySet* capSet)
    {
        CapabilitySet targetCaps = m_targetCaps;
        auto stageCapabilitySet = profile.getCapabilityName();
        CapabilitySet required(capSet->getCaps());

#if 0
        {
            StringBuilder sb;
            printDiagnosticArg(sb, required);
            fprintf(stderr, "required (full):    %s\n", sb.toString().getBuffer());

            sb.clear();
            printDiagnosticArg(sb, targetCaps);
            fprintf(stderr, "target (full):      %s\n", sb.toString().getBuffer());

            sb.clear();
            printDiagnosticArg(sb, stageCapabilitySet);
            fprintf(stderr, "stageCapabilitySet: %s\n", sb.toString().getBuffer());
        }
#endif

        targetCaps.join(stageCapabilitySet);

#if 0
        {
            StringBuilder sb;
            printDiagnosticArg(sb, targetCaps);
            fprintf(stderr, "target (stage):     %s\n", sb.toString().getBuffer());
        }
#endif

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
        {
            StringBuilder sb;
            printDiagnosticArg(sb, required);
            fprintf(stderr, "required (stage):   %s\n", sb.toString().getBuffer());

            sb.clear();
            printDiagnosticArg(sb, addedAtoms);
            fprintf(stderr, "diff:               %s\n", sb.toString().getBuffer());
        }
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

        m_sink->diagnose(capSet->sourceLoc, Diagnostics::seeCallOfFunc, "__requireCapability()");

        // we'll fail only if restrictive capability check was requested
        if (m_optionSet.getBoolOption(CompilerOptionName::RestrictiveCapabilityCheck))
            m_status = SLANG_FAIL;
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
                                IRCapabilitySet* capSet =
                                    as<IRCapabilitySet>(lateRequireCap->getCapabilitySet());
                                checkCapability(entryPoint, entryPointDecor->getProfile(), capSet);
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
