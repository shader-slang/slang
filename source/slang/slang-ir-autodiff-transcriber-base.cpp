// slang-ir-autodiff-trascriber-base.cpp
#include "slang-ir-autodiff-transcriber-base.h"

#include "slang-ir-autodiff.h"
#include "slang-ir-clone.h"
#include "slang-ir-dce.h"
#include "slang-ir-eliminate-phis.h"
#include "slang-ir-inst-pass-base.h"
#include "slang-ir-util.h"

namespace Slang
{

DiagnosticSink* AutoDiffTranscriberBase::getSink()
{
    SLANG_ASSERT(sink);
    return sink;
}


// TODO: (AD2 overhaul) Ensure that the primal

// orig -> primal
// orig -> diff

// origFunc
// {
//    inst-annotation(orig, diff-in-orig-context)
// }
//
// fwdDifffunc
// {
//    inst-annotation(primal, diff-in-primal-context)
// }
// cloned()


static void emitAnnotationsFromWitnessTable(
    DifferentiableTypeConformanceContext* context,
    IRBuilder* builder,
    IRInst* targetInst,
    IRInst* witnessTable)
{
    auto diffType = _lookupWitness(
        builder,
        witnessTable,
        context->sharedContext->differentialAssocTypeStructKey,
        builder->getTypeType());
    {
        IRInst* args[] = {
            targetInst,
            builder->getIntValue((IRIntegerValue)ValAssociationKind::DifferentialType),
            diffType};
        builder->emitIntrinsicInst(builder->getVoidType(), kIROp_AssociatedInstAnnotation, 3, args);
        context->associationCache.remove({targetInst, ValAssociationKind::DifferentialType});
    }

    auto diffDZeroMethod = _lookupWitness(
        builder,
        witnessTable,
        context->sharedContext->zeroMethodStructKey,
        context->sharedContext->zeroMethodType);

    {
        IRInst* args[] = {
            targetInst,
            builder->getIntValue((IRIntegerValue)ValAssociationKind::DifferentialZero),
            diffDZeroMethod};
        builder->emitIntrinsicInst(builder->getVoidType(), kIROp_AssociatedInstAnnotation, 3, args);
        context->associationCache.remove({targetInst, ValAssociationKind::DifferentialZero});
    }

    auto diffDAddMethod = _lookupWitness(
        builder,
        witnessTable,
        context->sharedContext->addMethodStructKey,
        context->sharedContext->addMethodType);
    {
        IRInst* args[] = {
            targetInst,
            builder->getIntValue((IRIntegerValue)ValAssociationKind::DifferentialAdd),
            diffDAddMethod};
        builder->emitIntrinsicInst(builder->getVoidType(), kIROp_AssociatedInstAnnotation, 3, args);
        context->associationCache.remove({targetInst, ValAssociationKind::DifferentialAdd});
    }

    auto diffPairType = builder->getDifferentialPairType((IRType*)targetInst, witnessTable);
    {
        IRInst* args[] = {
            targetInst,
            builder->getIntValue((IRIntegerValue)ValAssociationKind::DifferentialPairType),
            diffPairType};
        builder->emitIntrinsicInst(builder->getVoidType(), kIROp_AssociatedInstAnnotation, 3, args);
        context->associationCache.remove({targetInst, ValAssociationKind::DifferentialPairType});
    }
}

static void addTypeAnnotationsForHigherOrderDiff(
    IRBuilder* builder,
    DifferentiableTypeConformanceContext* differentiableTypeConformanceContext,
    IRType* origType,
    IRType* diffType)
{
    // Check if the diffType already has an annotation.
    if (!differentiableTypeConformanceContext->isDifferentiableType(diffType))
    {
        // TODO: lower the witness as a sepearate annotation rather than going through the pair.
        auto diffWitness = as<IRDifferentialPairType>(
                               differentiableTypeConformanceContext->tryGetAssociationOfKind(
                                   origType,
                                   ValAssociationKind::DifferentialPairType))
                               ->getWitness();
        if (as<IRStructKey>(diffWitness))
        {
            // For now, we'll ignore this case (happens for existential types)
            // Existential types cannot be higher-order differentiated at the moment.
            return;
        }

        // Differential : IDifferentiable
        auto diffWitnessDiffWitness = _lookupWitness(
            builder,
            diffWitness,
            differentiableTypeConformanceContext->sharedContext
                ->differentialAssocTypeWitnessStructKey,
            builder->getWitnessTableType(
                differentiableTypeConformanceContext->sharedContext->differentiableInterfaceType));

        emitAnnotationsFromWitnessTable(
            differentiableTypeConformanceContext,
            builder,
            diffType,
            diffWitnessDiffWitness);
    }

    auto pairType =
        as<IRDifferentialPairType>(differentiableTypeConformanceContext->tryGetAssociationOfKind(
            origType,
            ValAssociationKind::DifferentialPairType));
    if (pairType && !differentiableTypeConformanceContext->isDifferentiableType(pairType))
    {
        IRInst* args[] = {pairType};
        auto synWitnessTable = builder->emitIntrinsicInst(
            builder->getWitnessTableType(
                differentiableTypeConformanceContext->sharedContext->differentiableInterfaceType),
            kIROp_MakeIDifferentiableWitness,
            1,
            args);

        emitAnnotationsFromWitnessTable(
            differentiableTypeConformanceContext,
            builder,
            pairType,
            synWitnessTable);
    }
}

IRType* AutoDiffTranscriberBase::differentiateType(IRBuilder* builder, IRType* origType)
{
    if (isNoDiffType(origType))
        return nullptr;

    if (auto origPtrType = asRelevantPtrType(origType))
    {
        if (auto diffValueType = differentiateType(builder, origPtrType->getValueType()))
            return builder->getPtrTypeWithAddressSpace(diffValueType, origPtrType);
        else
            return nullptr;
    }

    if (auto diffValueType = differentiableTypeConformanceContext.tryGetAssociationOfKind(
            origType,
            ValAssociationKind::DifferentialType))
    {
        addTypeAnnotationsForHigherOrderDiff(
            builder,
            &differentiableTypeConformanceContext,
            origType,
            (IRType*)diffValueType);
        return (IRType*)diffValueType;
    }
    else if (
        auto diffPtrType = differentiableTypeConformanceContext.tryGetAssociationOfKind(
            origType,
            ValAssociationKind::DifferentialPtrType))
    {
        // TODO: Handle higher-order ptr types (should just need to add annotations similar to the
        // value case)
        return (IRType*)diffPtrType;
    }
    else
        return nullptr;
}

bool AutoDiffTranscriberBase::isExistentialType(IRType* type)
{
    switch (type->getOp())
    {
    case kIROp_ExtractExistentialType:
    case kIROp_InterfaceType:
    case kIROp_AssociatedType:
    case kIROp_LookupWitnessMethod:
        return true;
    default:
        return false;
    }
}

void AutoDiffTranscriberBase::copyOriginalDecorations(IRInst* origFunc, IRInst* diffFunc)
{
    for (auto decor : origFunc->getDecorations())
    {
        switch (decor->getOp())
        {
        case kIROp_ForceInlineDecoration:
            cloneDecoration(decor, diffFunc);
            break;
        }
    }
}

IRType* AutoDiffTranscriberBase::tryGetDiffPairType(IRBuilder* builder, IRType* originalType)
{
    // In case we're dealing with a parameter type, we need to split out the direction first.
    // If we're not, then the split & merge are no-ops.
    //
    auto [passingMode, baseType] = splitParameterDirectionAndType(originalType);
    auto basePairType = differentiableTypeConformanceContext.tryGetAssociationOfKind(
        baseType,
        ValAssociationKind::DifferentialPairType);

    if (!basePairType)
    {
        basePairType = differentiableTypeConformanceContext.tryGetAssociationOfKind(
            baseType,
            ValAssociationKind::DifferentialPtrPairType);
    }

    if (!basePairType)
        return nullptr;

    return fromDirectionAndType(builder, passingMode, (IRType*)basePairType);
}


void AutoDiffTranscriberBase::markDiffTypeInst(
    IRBuilder* builder,
    IRInst* diffInst,
    IRType* primalType)
{
    // Ignore module-level insts.
    if (as<IRModuleInst>(diffInst->getParent()))
        return;

    // Also ignore generic-container-level insts.
    if (as<IRBlock>(diffInst->getParent()) && as<IRGeneric>(diffInst->getParent()->getParent()))
        return;

    // TODO: This logic is a bit of a hack. We need to determine if the type is
    // relevant to ptr-type computation or not, or more complex applications
    // that use dynamic dispatch + ptr types will fail.
    //
    if (as<IRType>(diffInst))
    {
        builder->markInstAsDifferential(diffInst, nullptr);
        return;
    }

    SLANG_ASSERT(diffInst);
    SLANG_ASSERT(primalType);

    if (differentiableTypeConformanceContext.isDifferentiableValueType(primalType))
    {
        builder->markInstAsDifferential(diffInst, primalType);
    }
    else if (differentiableTypeConformanceContext.isDifferentiablePtrType(primalType))
    {
        builder->markInstAsPrimal(diffInst);
    }
    else
    {
        // Stop-gap solution to go with differential inst for now.
        builder->markInstAsDifferential(diffInst, primalType);
    }
}

void AutoDiffTranscriberBase::markDiffPairTypeInst(
    IRBuilder* builder,
    IRInst* diffPairInst,
    IRType* pairType)
{
    SLANG_ASSERT(diffPairInst);
    SLANG_ASSERT(pairType);
    SLANG_ASSERT(as<IRDifferentialPairTypeBase>(pairType));

    if (as<IRDifferentialPairType>(pairType))
    {
        builder->markInstAsMixedDifferential(diffPairInst, pairType);
    }
    else if (as<IRDifferentialPtrPairType>(pairType))
    {
        builder->markInstAsPrimal(diffPairInst);
    }
    else
    {
        SLANG_UNEXPECTED("unexpected differentiable type");
    }
}


} // namespace Slang
