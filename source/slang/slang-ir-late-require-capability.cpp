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

/// Return every atom that `after` has and `before` does not, comparing the two capability sets
/// one (target, stage) pair at a time.
///
/// Both sets hold a conjunction of atoms per pair, and what this warning wants to name is the
/// atoms added for the pair being compiled. Taking the first conjunction of each set, as this
/// once did, asks the two hash maps for whichever pair they happen to yield first, and not
/// necessarily the same pair from each, since they are separate maps. The atoms named then
/// changed from one run of the compiler to the next, and could belong to a target the user was
/// not compiling for: an HLSL entry point needing `cooperative_vector` was reported as upgrading
/// its profile to 'GL_NV_cooperative_vector' rather than 'sm_6_9'. Pairing the conjunctions up by
/// target and stage and taking the union of the differences gives the same answer whatever order
/// the pairs are visited in.
///
/// NOTE: `slang-check-shader.cpp` has the same helper for the same reason. They belong together
/// in `slang-capability.h`, which is left for a change that is allowed to rebuild everything
/// that header reaches.
static CapabilityAtomSet _getAtomsAddedToCapabilities(
    const CapabilitySet& before,
    const CapabilitySet& after)
{
    CapabilityAtomSet addedAtoms;
    for (const auto& afterTarget : after.getCapabilityTargetSets())
    {
        auto beforeTarget = before.getCapabilityTargetSets().tryGetValue(afterTarget.first);
        if (!beforeTarget)
            continue;

        for (const auto& afterStage : afterTarget.second.getShaderStageSets())
        {
            auto beforeStage = beforeTarget->getShaderStageSets().tryGetValue(afterStage.first);
            if (!beforeStage || !beforeStage->atomSet || !afterStage.second.atomSet)
                continue;

            CapabilityAtomSet difference;
            CapabilityAtomSet::calcSubtract(
                difference,
                afterStage.second.atomSet.value(),
                beforeStage->atomSet.value());
            addedAtoms.add(difference);
        }
    }
    return addedAtoms;
}

struct ProcessLateRequireCapabilityInstsContext
{
    IRModule* const m_module;
    CapabilitySet m_targetCaps;
    CompilerOptionSet& m_optionSet;
    DiagnosticSink* const m_sink;

    Dictionary<IRInst*, HashSet<IRFunc*>> m_mapInstToReferencingEntryPoints;

    // entry point --> diagnosed capability strings
    Dictionary<IRFunc*, HashSet<String>> m_diagnosedCapsStrs;

    ProcessLateRequireCapabilityInstsContext(
        IRModule* module,
        const CapabilitySet& targetCaps,
        CompilerOptionSet& optionSet,
        DiagnosticSink* sink)
        : m_module(module), m_targetCaps(targetCaps), m_optionSet(optionSet), m_sink(sink)
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

        // check that we have the required caps for this stage
        if (stageTargetCaps.atLeastOneSetImpliedInOther(required) ==
            CapabilitySet::ImpliesReturnFlags::Implied)
            return;

        // figure out the missing delta
        CapabilityAtomSet addedAtoms = _getAtomsAddedToCapabilities(stageTargetCaps, required);

        sb.clear();
        printDiagnosticArg(sb, addedAtoms);
        String missingCapsStr = sb.toString();

        // Add if not already added
        if (!m_diagnosedCapsStrs[entry].add(missingCapsStr))
            return; // already added, don't diagnose again

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

                    if (m_optionSet.getBoolOption(CompilerOptionName::IgnoreCapabilities))
                        continue;

                    const HashSet<IRFunc*>* entryPoints =
                        m_mapInstToReferencingEntryPoints.tryGetValue(func);

                    if (!entryPoints)
                        continue;

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
        codeGenContext->getTargetReq()->getOptionSet(),
        sink);

    context.processModule();
}

} // namespace Slang
