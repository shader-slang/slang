// slang-ir-late-require-capability.cpp

#include "slang-ir-late-require-capability.h"

#include "slang-ir-call-graph.h"
#include "slang-ir-dce.h"
#include "slang-ir-specialize.h"
#include "slang-ir-translate.h"
#include "slang.h"
#include "slang/ir/slang-ir-insts.h"
#include "slang/ir/slang-ir-util.h"
#include "slang/ir/slang-ir.h"
#include "slang/semantic-checking/slang-check-impl.h"
#include "slang/slang-profile.h"
#include "slang/slang-target.h"

namespace Slang
{

struct MaterializeEnumRequirementsContext
{
    IRModule* module;
    EnumRequirementMaterializationPhase phase;

    // Native types retain their generic applications for target emission. Give their checks a
    // separate generic function so normal specialization can substitute N in this example:
    //
    //     __target_intrinsic(hlsl, "int")
    //     struct Wrapper<let N : int> { Holder<Mode(N)> field; }
    //
    // hoistValueFromGeneric preserves the exact generic parameters, including enclosing
    // generics. Its prefix clone also retains any constexpr operands needed by the checks.
    // The validator is a metadata template containing only checks and a return; it is never
    // called by generated code, and the original native type is never specialized by this pass.
    void createIntrinsicTypeValidators()
    {
        List<IRInst*> workList;
        workList.add(module->getModuleInst());
        for (Index i = 0; i < workList.getCount(); ++i)
            for (auto child : workList[i]->getDecorationsAndChildren())
                workList.add(child);

        for (auto inst : workList)
        {
            auto generic = as<IRGeneric>(inst);
            if (!generic || generic->findDecoration<IREnumRequirementValidatorDecoration>())
                continue;
            auto owner = as<IRStructType>(findGenericReturnVal(generic));
            if (!owner || !owner->findDecoration<IRTargetIntrinsicDecoration>())
                continue;
            if (!owner->getFirstChild() &&
                !owner->findDecoration<IREnumCastRequirementDecoration>())
                continue;

            IRBuilder builder(module);
            builder.setInsertBefore(generic->getFirstBlock()->getTerminator());
            auto validator = builder.createFunc();
            validator->setFullType(builder.getFuncType(0, nullptr, builder.getVoidType()));
            builder.setInsertInto(validator);
            builder.emitBlock();
            for (auto decoration : owner->getDecorations())
                if (as<IREnumCastRequirementDecoration>(decoration))
                    emitCheck(builder, decoration);
            for (auto field : owner->getFields())
            {
                IRInst* source = field->getFieldType();
                builder.emitIntrinsicInst(
                    builder.getVoidType(),
                    kIROp_CheckEnumRequirements,
                    1,
                    &source);
            }
            builder.emitReturn();
            IRInst* unusedSpecialization = nullptr;
            auto validatorGeneric =
                cast<IRGeneric>(hoistValueFromGeneric(builder, validator, unusedSpecialization));
            validator->removeAndDeallocate();
            builder.addDecoration(
                generic,
                kIROp_EnumRequirementValidatorDecoration,
                validatorGeneric);
        }
    }

    // Read only the compiler-owned validator grammar. Reusing the same visited set as the type
    // walk bounds recursive fields, and copying its checks emits no runtime helper or call.
    bool collectValidatorRequirements(
        IRSpecialize* source,
        IRGeneric* validator,
        HashSet<IRInst*>& visited,
        List<IRInst*>& requirements)
    {
        IRBuilder builder(module);
        builder.setInsertBefore(source);
        List<IRInst*> args;
        for (UInt i = 0; i < source->getArgCount(); ++i)
            args.add(source->getArg(i));
        auto specialization = cast<IRSpecialize>(builder.emitSpecializeInst(
            builder.getFuncType(0, nullptr, builder.getVoidType()),
            validator,
            args));
        auto function = cast<IRFunc>(specializeGeneric(specialization));
        SLANG_RELEASE_ASSERT(function && function->getFirstBlock() == function->getLastBlock());
        bool unresolved = false;
        for (auto instruction : function->getFirstBlock()->getOrdinaryInsts())
        {
            switch (instruction->getOp())
            {
            case kIROp_CheckEnumCast:
                requirements.add(instruction);
                break;
            case kIROp_CheckEnumRequirements:
                unresolved |= collectRequirements(
                    cast<IRCheckEnumRequirements>(instruction)->getSource(),
                    visited,
                    requirements);
                break;
            case kIROp_Return:
                break;
            default:
                SLANG_UNEXPECTED("invalid enum requirement validator instruction");
            }
        }
        return unresolved;
    }

    // Follow declaration metadata and type structure, never user function bodies. Consider
    // `Wrapper<N> value;` where one field has type `Holder<Mode(N)>`: specialization substitutes
    // N in the wrapper's decoration, and this walk finds that declaration-owned obligation even
    // if the program only reads another field. An unresolved specialization must retain its
    // anchor until its concrete type is available.
    bool collectRequirements(IRInst* source, HashSet<IRInst*>& visited, List<IRInst*>& requirements)
    {
        if (!source || !visited.add(source))
            return false;
        if (auto specialization = as<IRSpecialize>(source))
        {
            // The first pass records source uses before linking or generic specialization.
            // Keep this anchor until its arguments and the specialization dictionary are ready.
            if (phase == EnumRequirementMaterializationPhase::BeforeSpecialization)
                return true;
            if (auto generic = as<IRGeneric>(specialization->getBase()))
                if (auto validator =
                        generic->findDecoration<IREnumRequirementValidatorDecoration>())
                    return collectValidatorRequirements(
                        specialization,
                        validator->getValidator(),
                        visited,
                        requirements);
            return true;
        }
        if (as<IRLookupWitnessMethod>(source))
            return true;

        for (auto decoration : source->getDecorations())
            if (as<IREnumCastRequirementDecoration>(decoration))
                requirements.add(decoration);

        bool unresolved = false;
        auto dataType = source->getDataType();
        bool isType = as<IRType>(source) || (dataType && dataType->getOp() == kIROp_TypeKind);
        if (isType)
        {
            if (as<IRParam>(source))
                return true;
            if (auto structType = as<IRStructType>(source))
            {
                for (auto field : structType->getFields())
                    unresolved |= collectRequirements(field->getFieldType(), visited, requirements);
            }
            for (UInt i = 0; i < source->getOperandCount(); ++i)
            {
                auto operand = source->getOperand(i);
                auto operandType = operand->getDataType();
                if (as<IRType>(operand) || (operandType && operandType->getOp() == kIROp_TypeKind))
                    unresolved |= collectRequirements(operand, visited, requirements);
            }
        }
        else
        {
            unresolved |= collectRequirements(source->getFullType(), visited, requirements);
        }
        return unresolved;
    }

    void emitCheck(IRBuilder& builder, IRInst* requirement)
    {
        List<IRInst*> operands;
        for (UInt i = 0; i < requirement->getOperandCount(); ++i)
            operands.add(requirement->getOperand(i));
        IRBuilderSourceLocRAII sourceLoc(&builder, requirement->sourceLoc);
        builder.emitIntrinsicInst(
            builder.getVoidType(),
            kIROp_CheckEnumCast,
            operands.getCount(),
            operands.getBuffer());
    }

    void processModule()
    {
        bool beforeSpecialization =
            phase == EnumRequirementMaterializationPhase::BeforeSpecialization;
        if (beforeSpecialization)
            createIntrinsicTypeValidators();
        List<IRInst*> workList;
        List<IRCheckEnumRequirements*> anchors;
        List<IRFunc*> functions;
        List<IRInst*> decorationsToRemove;
        workList.add(module->getModuleInst());
        for (Index i = 0; i < workList.getCount(); ++i)
        {
            auto inst = workList[i];
            for (auto child : inst->getDecorationsAndChildren())
                workList.add(child);
            if (auto anchor = as<IRCheckEnumRequirements>(inst))
                anchors.add(anchor);
            if (auto func = as<IRFunc>(inst); beforeSpecialization && func && func->getFirstBlock())
                functions.add(func);
        }

        for (auto func : functions)
        {
            // A function's signature is used whenever the function executes. Materialize
            // its header checks before early inlining can discard the declaration itself.
            IRBuilder builder(module);
            builder.setInsertBefore(func->getFirstBlock()->getFirstOrdinaryInst());
            for (auto decoration : func->getDecorations())
            {
                if (as<IREnumCastRequirementDecoration>(decoration))
                {
                    emitCheck(builder, decoration);
                    decorationsToRemove.add(decoration);
                }
            }
            IRInst* source = func->getFullType();
            auto anchor = cast<IRCheckEnumRequirements>(builder.emitIntrinsicInst(
                builder.getVoidType(),
                kIROp_CheckEnumRequirements,
                1,
                &source));
            anchor->sourceLoc = func->sourceLoc;
            anchors.add(anchor);
        }

        for (auto anchor : anchors)
        {
            if (!beforeSpecialization && findOuterGeneric(anchor))
                continue;
            HashSet<IRInst*> visited;
            List<IRInst*> requirements;
            bool unresolved = collectRequirements(anchor->getSource(), visited, requirements);
            if (unresolved && phase != EnumRequirementMaterializationPhase::Final)
                continue;
            IRBuilder builder(module);
            builder.setInsertBefore(anchor);
            for (auto requirement : requirements)
                emitCheck(builder, requirement);
            anchor->removeAndDeallocate();
        }
        for (auto decoration : decorationsToRemove)
            decoration->removeAndDeallocate();
    }
};

void materializeEnumRequirements(IRModule* module, EnumRequirementMaterializationPhase phase)
{
    // The final checkpoint runs after the main specialization dictionary has been released.
    // Native-type validators still use the ordinary specialization cache, so give this pass
    // its own dictionary when there is no existing one to share.
    bool ownsDictionary = phase != EnumRequirementMaterializationPhase::BeforeSpecialization &&
                          !module->getTranslationDict();
    if (ownsDictionary)
        initializeTranslationDictionary(module);
    SLANG_DEFER({
        if (ownsDictionary)
            clearTranslationDictionary(module);
    });

    MaterializeEnumRequirementsContext context{module, phase};
    context.processModule();
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
        HashSet<IRInstKey> checkedEnumCasts;

        // scan the function for IRLateRequireCapability instructions
        for (auto block : func->getBlocks())
        {
            for (auto inst : block->getOrdinaryInsts())
            {
                if (auto enumCast = as<IRCheckEnumCast>(inst))
                {
                    // Several source uses can materialize the same declaration obligation.
                    // Keep those uses through optimization, but report identical checks once
                    // per function using the ordinary IR instruction key.
                    if (checkedEnumCasts.add(IRInstKey{enumCast}))
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
    // Higher-order specialization can expose further anchors after the main specialization
    // checkpoint. Their operands keep the owner metadata alive until this final expansion.
    materializeEnumRequirements(module, EnumRequirementMaterializationPhase::Final);
    ProcessLateRequireCapabilityInstsContext context(
        module,
        codeGenContext->getTargetCaps(),
        codeGenContext->getTargetReq()->getOptionSet(),
        sink);

    context.processModule();

    // Declaration metadata remains available through all specialization passes. Every surviving
    // source use has now been checked, so it has no runtime meaning and can release its case data.
    List<IRInst*> workList;
    List<IRInst*> decorations;
    workList.add(module->getModuleInst());
    for (Index i = 0; i < workList.getCount(); ++i)
    {
        for (auto child : workList[i]->getDecorationsAndChildren())
            workList.add(child);
        if (as<IREnumCastRequirementDecoration>(workList[i]) ||
            as<IREnumRequirementValidatorDecoration>(workList[i]))
            decorations.add(workList[i]);
    }
    for (auto decoration : decorations)
        decoration->removeAndDeallocate();

    // Case tables are compiler metadata used only by the checks removed above.
    eliminateDeadCode(module);
}

} // namespace Slang
