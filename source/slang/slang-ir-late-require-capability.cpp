// slang-ir-late-require-capability.cpp

#include "slang-ir-late-require-capability.h"

#include "slang-check-impl.h"
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

    Dictionary<IRInst*, HashSet<IRFunc*>> m_mapInstToReferencingEntryPoints;

    // entry point --> diagnosed capability strings
    Dictionary<IRFunc*, HashSet<String>> m_diagnosedCapsStrs;

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
        Profile profile,
        IRLateRequireCapability* irInst,
        IRCapabilitySet* capSet)
    {
        CapabilitySet stageTargetCaps = m_targetCaps;
        CapabilitySet stageCapabilitySet = profile.getCapabilityName();
        CapabilitySet required(capSet->getCaps());
        StringBuilder sb;

        stageTargetCaps.join(stageCapabilitySet);
        required.join(stageCapabilitySet);

#if 0
        {
            sb.clear();
            printDiagnosticArg(sb, capSet->getCaps());
            fprintf(stderr, "required (full):    %s\n", sb.toString().getBuffer());

            sb.clear();
            printDiagnosticArg(sb, m_targetCaps);
            fprintf(stderr, "target (full):      %s\n", sb.toString().getBuffer());

            sb.clear();
            printDiagnosticArg(sb, profile.getCapabilityName());
            fprintf(stderr, "stageCapabilitySet: %s\n", sb.toString().getBuffer());

            sb.clear();
            printDiagnosticArg(sb, stageTargetCaps);
            fprintf(stderr, "target (stage):     %s\n", sb.toString().getBuffer());
        }
#endif

        // check that we have the required caps for this stage
        if (stageTargetCaps.atLeastOneSetImpliedInOther(required) ==
            CapabilitySet::ImpliesReturnFlags::Implied)
            return;


        // figure out the missing delta
        CapabilityAtomSet addedAtoms{};

        if (auto stageCapSet = stageTargetCaps.getAtomSets())
        {
            if (auto requiredSet = required.getAtomSets())
            {
                CapabilityAtomSet::calcSubtract(addedAtoms, (*requiredSet), (*stageCapSet));
            }
        }

        sb.clear();
        printDiagnosticArg(sb, addedAtoms);
        String missingCapsStr = sb.toString();

        // Add if not already added
        if (!m_diagnosedCapsStrs[entry].add(missingCapsStr))
            return; // already added, don't diagnose again

#if 0
        {
            sb.clear();
            printDiagnosticArg(sb, required);
            fprintf(stderr, "required (stage):   %s\n", sb.toString().getBuffer());

            sb.clear();
            fprintf(stderr, "diff:               %s\n", missingCapsStr.getBuffer());
        }
#endif

        sb.clear();
        printDiagnosticArg(sb, entry);
        String entryName = sb.toString();

        maybeDiagnoseWarningOrError(
            m_sink,
            m_optionSet,
            DiagnosticCategory::Capability,
            Diagnostics::ProfileImplicitlyUpgraded{
                .entryPoint = entryName,
                .profile = m_optionSet.getProfile().getName(),
                .capabilities = missingCapsStr,
                .location = entry->sourceLoc,
            },
            Diagnostics::ProfileImplicitlyUpgradedRestrictive{
                .entryPoint = entryName,
                .profile = m_optionSet.getProfile().getName(),
                .capabilities = missingCapsStr,
                .location = entry->sourceLoc,
            });

        m_sink->diagnose(Diagnostics::SeeCallOfFunc{
            .name = "__requireCapability",
            .location = irInst->sourceLoc});
        diagnoseCallStack(irInst, m_sink);
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
                            if (IREntryPointDecoration* entryPointDecor =
                                    entryPoint->findDecoration<IREntryPointDecoration>())
                            {
                                IRCapabilitySet* capSet = lateRequireCap->getCapabilitySet();
                                checkCapability(
                                    entryPoint,
                                    entryPointDecor->getProfile(),
                                    lateRequireCap,
                                    capSet);
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

void processLateRequireCapabilityInsts(
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
}

} // namespace Slang
