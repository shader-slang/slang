// slang-ir-late-require-capability.cpp

#include "slang-ir-late-require-capability.h"

#include "slang-ir-call-graph.h"
#include "slang-ir-dce.h"
#include "slang.h"
#include "slang/ir/slang-ir-insts.h"
#include "slang/ir/slang-ir.h"
#include "slang/semantic-checking/slang-check-impl.h"
#include "slang/slang-profile.h"
#include "slang/slang-target.h"

namespace Slang
{

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
        IRInst* irInst,
        IRCapabilitySet* capSet,
        const char* requirementName = "__requireCapability")
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

        m_sink->diagnose(
            Diagnostics::SeeCallOfFunc{.name = requirementName, .location = irInst->sourceLoc});
        diagnoseCallStack(irInst, m_sink);
    }

    // Select the case after generic specialization has made the conversion's input concrete.
    // Consider `Mode choose<let N : int>() { return Mode(N + 1); }`. Lowering keeps the tag
    // calculation and this check separate, so returning or simplifying the numeric value does
    // not erase the selected case's capability requirement.
    void checkEnumCast(IRFunc* func, IRCheckEnumCast* inst)
    {
        auto value = inst->getValue();
        auto enumName = String(inst->getEnumName()->getStringSlice());
        if (!as<IRIntLit>(value) && !as<IRBoolLit>(value))
        {
            m_sink->diagnose(Diagnostics::EnumCastSpecializationNotConstant{
                .enumName = enumName,
                .location = inst->sourceLoc});
            diagnoseCallStack(inst, m_sink);
            return;
        }

        IREnumCaseInfo* selectedCase = nullptr;
        for (UInt i = 0; i < inst->getCaseCount(); ++i)
        {
            auto caseInfo = inst->getCase(i);
            // Lowering gives the input and every tag the enum's underlying integer type.
            // Canonical IR literals therefore identify the matching value without another
            // integer-conversion or equality implementation here.
            if (caseInfo->getTag() != value)
                continue;
            SLANG_RELEASE_ASSERT(!selectedCase);
            selectedCase = caseInfo;
        }
        if (!selectedCase)
        {
            m_sink->diagnose(Diagnostics::EnumCastSpecializationNoMatchingCase{
                .enumName = enumName,
                .location = inst->sourceLoc});
            diagnoseCallStack(inst, m_sink);
            return;
        }

        if (m_optionSet.getBoolOption(CompilerOptionName::IgnoreCapabilities))
            return;
        auto entryPoints = m_mapInstToReferencingEntryPoints.tryGetValue(func);
        if (!entryPoints)
            return;
        for (auto entryPoint : *entryPoints)
        {
            auto entryPointDecor = entryPoint->findDecoration<IREntryPointDecoration>();
            if (!entryPointDecor)
                continue;
            auto required = selectedCase->getCapabilitySet()->getCaps();
            auto targetCaps = m_targetCaps;
            targetCaps.join(entryPointDecor->getProfile().getCapabilityName());
            if (targetCaps.isIncompatibleWith(required))
            {
                StringBuilder capabilities;
                printDiagnosticArg(capabilities, required);
                m_sink->diagnose(Diagnostics::EnumCastSpecializationCapabilityUnavailable{
                    .enumCase = String(selectedCase->getCaseName()->getStringSlice()),
                    .enumName = enumName,
                    .capabilities = capabilities.produceString(),
                    .location = inst->sourceLoc});
                diagnoseCallStack(inst, m_sink);
            }
            else
            {
                checkCapability(
                    entryPoint,
                    entryPointDecor->getProfile(),
                    inst,
                    selectedCase->getCapabilitySet(),
                    "enum conversion");
            }
        }
    }

    void processFunc(IRFunc* func)
    {
        List<IRInst*> instsToRemove;

        // scan the function for IRLateRequireCapability instructions
        for (auto block : func->getBlocks())
        {
            for (auto inst : block->getOrdinaryInsts())
            {
                if (auto enumCast = as<IRCheckEnumCast>(inst))
                {
                    checkEnumCast(func, enumCast);
                    instsToRemove.add(enumCast);
                }
                else if (auto lateRequireCap = as<IRLateRequireCapability>(inst))
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

    // Case tables are compiler metadata used only by the checks removed above.
    eliminateDeadCode(module);
}

} // namespace Slang
