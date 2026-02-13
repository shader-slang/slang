// slang-ir-process-late-require-cap-insts.cpp

#include "slang-ir-process-late-require-cap-insts.h"

#include "slang-ir-call-graph.h"
#include "slang-ir-insts.h"
#include "slang-ir.h"
#include "slang-target.h"
#include "slang-profile.h"
#include "slang.h"

namespace Slang
{

struct ProcessLateRequireCapabilityInstsContext
{
    IRModule* m_module;
    CapabilitySet m_targetCaps;
    const CodeGenTarget m_target;
    DiagnosticSink* m_sink;

    Dictionary<IRInst*, HashSet<IRFunc*>> m_mapInstToReferencingEntryPoints;

    bool m_errorEncountered = false;

    ProcessLateRequireCapabilityInstsContext(
        IRModule* module, const CapabilitySet& targetCaps, CodeGenTarget target, DiagnosticSink* sink)
        : m_module(module), m_targetCaps(targetCaps), m_target(target), m_sink(sink)
    {
    }

    bool checkCapability(Stage stage, List<CapabilityName> capNames)
    {
        const CapabilityAtom targetAtom = m_targetCaps.getCompileTarget();
        const CapabilityAtom stageAtom = getAtomFromStage(stage);

        // build capability set with target/stage/no-caps (for joining)
        CapabilitySet stageSet;
        CapabilityTargetSet &tset = stageSet.getCapabilityTargetSets()[targetAtom];
        tset.target = targetAtom;
        tset.shaderStageSets[stageAtom].stage = stageAtom;

        // grab the stage caps from the target caps by joining
        CapabilitySet stageCaps(m_targetCaps);
        stageCaps.join(stageSet);
        CapabilitySet required(capNames);

        // check that we have the required caps for this stage
        return stageCaps.atLeastOneSetImpliedInOther(required) == CapabilitySet::ImpliesReturnFlags::Implied;
    }

    void processCapability(IRFunc* entry, Stage stage, IRLateRequireCapability* inst)
    {
        List<CapabilityName> capNames;

        for (UInt i = 0; i < inst->getOperandCount(); ++i)
        {
            IRConstant* capConstant = as<IRConstant>(inst->getOperand(i));
            if (!capConstant)
            {
                m_sink->diagnose(
                    inst,
                    Diagnostics::expectedAStringLiteral);
                m_errorEncountered = true;
                return;
            }

            CapabilityName capName = findCapabilityName(capNameStr->getStringSlice());
            if (capName == CapabilityName::Invalid)
            {
                m_sink->diagnose(
                    inst,
                    Diagnostics::unknownCapability,
                    capNameStr);
                m_errorEncountered = true;
                return;
            }

            capNames.add(capName);
        }

        if (!checkCapability(stage, capNames))
        {
            m_sink->diagnose(
                inst,
                Diagnostics::entryPointUsesUnavailableCapability,
                entry,
                m_target,
                stage);
            m_errorEncountered = true;
        }
    }

    void processFunc(IRFunc* func)
    {
        List<IRLateRequireCapability *> instsToRemove;

        // scan the function for IRLateRequireCapability instructions
        for (auto block : func->getBlocks())
        {
            for (auto inst : block->getOrdinaryInsts())
            {
                if (auto lateRequireCap = as<IRLateRequireCapability>(inst))
                {
                    instsToRemove.add(lateRequireCap);

                    const HashSet<IRFunc*> *entryPoints =
                        m_mapInstToReferencingEntryPoints.tryGetValue(func);

                    if (entryPoints)
                    {
                        for (auto entryPoint : *entryPoints)
                        {
                            if (auto entryPointDecor = entryPoint->findDecoration<IREntryPointDecoration>())
                            {
                                auto stage = entryPointDecor->getProfile().getStage();
                                processCapability(entryPoint, stage, lateRequireCap);
                            }
                            else if (entryPoint->findDecoration<IRCudaKernelDecoration>())
                            {
                                // CUDA entrypoint is treated as a compute stage
                                processCapability(entryPoint, Stage::Compute, lateRequireCap);
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
        // Collect all functions that need processing.
        // Process all callee's before callers; otherwise we introduce bugs

        buildEntryPointReferenceGraph(m_mapInstToReferencingEntryPoints, m_module);
        HashSet<IRFunc*> visitedFunctions;

        for (auto inst = m_module->getModuleInst()->getFirstChild(); inst; inst = inst->getNextInst())
        {
            auto func = as<IRFunc>(inst);
            if (!func)
                continue;

            if (visitedFunctions.contains(func))
                continue;

            processFunc(func);
            visitedFunctions.add(func);
        }
    }
};

SlangResult processLateRequireCapabilityInsts(IRModule* module, CodeGenContext* codeGenContext, DiagnosticSink* sink)
{
    ProcessLateRequireCapabilityInstsContext context(
        module, codeGenContext->getTargetCaps(), codeGenContext->getTargetFormat(), sink);

    context.processModule();
    return context.m_errorEncountered ? SLANG_OK : SLANG_FAIL;
}

} // namespace Slang
