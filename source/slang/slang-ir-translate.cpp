#include "slang-ir-translate.h"

#include "slang-ir-insts.h"
#include "slang-ir-util.h"
#include "slang-ir.h"

namespace Slang
{

IRInst* TranslationContext::maybeTranslateInst(IRInst* inst)
{
    if (auto translatedInst = tryGetTranslation(irModule, inst))
        return translatedInst;

    auto memoize = [&](IRInst* result)
    {
        registerTranslation(irModule, inst, result);
        return result;
    };

    IRInst* translationResult = nullptr;
    IRBuilder subBuilder(inst->getModule());
    subBuilder.setInsertBefore(inst);
    switch (inst->getOp())
    {
    case kIROp_BackwardDifferentiate:
        {
            // translationResult =
            //     fwdDiffTranslator.processTranslationRequest(inst, &autodiffContext, sink);
            return memoize(maybeTranslateBackwardDerivative(
                &autodiffContext,
                sink,
                cast<IRBackwardDifferentiate>(inst)));
        }
        break;
    case kIROp_TrivialBackwardDifferentiate:
        {
            return memoize(maybeTranslateTrivialBackwardDerivative(
                &autodiffContext,
                sink,
                cast<IRTrivialBackwardDifferentiate>(inst)));
        }
        break;
    case kIROp_ForwardDifferentiate:
        {
            return memoize(maybeTranslateForwardDerivative(
                &autodiffContext,
                sink,
                cast<IRForwardDifferentiate>(inst)));
        }
        break;
    case kIROp_TrivialForwardDifferentiate:
        {
            return memoize(maybeTranslateTrivialForwardDerivative(
                &autodiffContext,
                sink,
                cast<IRTrivialForwardDifferentiate>(inst)));
        }
        break;
    case kIROp_BackwardFromLegacyBwdDiffFunc:
        {
            return memoize(maybeTranslateLegacyToNewBackwardDerivative(
                &autodiffContext,
                sink,
                cast<IRBackwardFromLegacyBwdDiffFunc>(inst)));
        }
    case kIROp_LegacyBackwardDifferentiate:
        {
            return memoize(maybeTranslateLegacyBackwardDerivative(
                &autodiffContext,
                sink,
                cast<IRLegacyBackwardDifferentiate>(inst)));
        }
    case kIROp_BackwardDifferentiatePrimal:
    case kIROp_BackwardDifferentiatePropagate:
    case kIROp_BackwardContextGetPrimalVal:
    case kIROp_BackwardDiffIntermediateContextType:
        {
            auto operand = inst->getOperand(0);

            auto bwdDiffInst =
                subBuilder.emitIntrinsicInst(nullptr, kIROp_BackwardDifferentiate, 1, &operand);

            // Translate the full 4-tuple result.
            auto translatedTuple = maybeTranslateInst(cast<IRBackwardDifferentiate>(bwdDiffInst));
            if (translatedTuple == bwdDiffInst)
                return memoize(bwdDiffInst);

            SLANG_ASSERT(as<IRMakeTuple>(translatedTuple));
            switch (inst->getOp())
            {
            case kIROp_BackwardDifferentiatePrimal:
                return memoize(as<IRMakeTuple>(translatedTuple)->getOperand(0));
            case kIROp_BackwardDifferentiatePropagate:
                return memoize(as<IRMakeTuple>(translatedTuple)->getOperand(1));
            case kIROp_BackwardContextGetPrimalVal:
                return memoize(as<IRMakeTuple>(translatedTuple)->getOperand(2));
            case kIROp_BackwardDiffIntermediateContextType:
                return memoize(as<IRMakeTuple>(translatedTuple)->getOperand(3));
            default:
                SLANG_UNEXPECTED("unhandled trivial backward differentiation case");
                break;
            }
        }
        break;
    case kIROp_TrivialBackwardDifferentiatePrimal:
    case kIROp_TrivialBackwardDifferentiatePropagate:
    case kIROp_TrivialBackwardContextGetPrimalVal:
    case kIROp_TrivialBackwardDiffIntermediateContextType:
        {
            auto operand = inst->getOperand(0);

            auto bwdDiffInst = subBuilder.emitIntrinsicInst(
                nullptr,
                kIROp_TrivialBackwardDifferentiate,
                1,
                &operand);

            // Translate the full 4-tuple result.
            auto translatedTuple =
                maybeTranslateInst(cast<IRTrivialBackwardDifferentiate>(bwdDiffInst));
            if (translatedTuple == bwdDiffInst)
                return memoize(bwdDiffInst);

            SLANG_ASSERT(as<IRMakeTuple>(translatedTuple));
            switch (inst->getOp())
            {
            case kIROp_TrivialBackwardDifferentiatePrimal:
                return memoize(as<IRMakeTuple>(translatedTuple)->getOperand(0));
            case kIROp_TrivialBackwardDifferentiatePropagate:
                return memoize(as<IRMakeTuple>(translatedTuple)->getOperand(1));
            case kIROp_TrivialBackwardContextGetPrimalVal:
                return memoize(as<IRMakeTuple>(translatedTuple)->getOperand(2));
            case kIROp_TrivialBackwardDiffIntermediateContextType:
                return memoize(as<IRMakeTuple>(translatedTuple)->getOperand(3));
            default:
                SLANG_UNEXPECTED("unhandled trivial backward differentiation case");
                break;
            }
        }
        break;
    case kIROp_FunctionCopy:
        {
            auto funcOperand = inst->getOperand(0);
            translationResult = funcOperand;
        }
        break;
    case kIROp_BackwardContextFromLegacyBwdDiffFunc:
    case kIROp_BackwardPrimalFromLegacyBwdDiffFunc:
    case kIROp_BackwardPropagateFromLegacyBwdDiffFunc:
    case kIROp_BackwardContextGetValFromLegacyBwdDiffFunc:
        {
            auto targetFunc = inst->getOperand(0);
            auto bwdDiffFunc = inst->getOperand(1);

            List<IRInst*> args;
            args.add(targetFunc);
            args.add(bwdDiffFunc);

            auto legacyToNewBwdDiffInst = subBuilder.emitIntrinsicInst(
                nullptr,
                kIROp_BackwardFromLegacyBwdDiffFunc,
                args.getCount(),
                args.getBuffer());

            // Translate the full 4-tuple result.
            auto translatedTuple =
                maybeTranslateInst(cast<IRBackwardFromLegacyBwdDiffFunc>(legacyToNewBwdDiffInst));
            if (translatedTuple == legacyToNewBwdDiffInst)
                return memoize(legacyToNewBwdDiffInst);

            SLANG_ASSERT(as<IRMakeTuple>(translatedTuple));
            switch (inst->getOp())
            {
            case kIROp_BackwardPrimalFromLegacyBwdDiffFunc:
                return memoize(as<IRMakeTuple>(translatedTuple)->getOperand(0));
            case kIROp_BackwardPropagateFromLegacyBwdDiffFunc:
                return memoize(as<IRMakeTuple>(translatedTuple)->getOperand(1));
            case kIROp_BackwardContextGetValFromLegacyBwdDiffFunc:
                return memoize(as<IRMakeTuple>(translatedTuple)->getOperand(2));
            case kIROp_BackwardContextFromLegacyBwdDiffFunc:
                return memoize(as<IRMakeTuple>(translatedTuple)->getOperand(3));
            default:
                SLANG_UNEXPECTED("unhandled trivial backward differentiation case");
                break;
            }
        }
        break;
    // For higher order differentiation, we can synthesize new tables for
    // conformance to IForwardDifferentiable and IBackwardDifferentiable
    //
    case kIROp_SynthesizedForwardDerivativeWitnessTable:
        {
            return memoize(maybeTranslateForwardDerivativeWitness(
                &autodiffContext,
                sink,
                cast<IRSynthesizedForwardDerivativeWitnessTable>(inst)));
        }
        break;
    case kIROp_SynthesizedBackwardDerivativeWitnessTable:
        {
            SLANG_UNEXPECTED("not implemented yet");
        }
        break;
    case kIROp_MakeIDifferentiableWitness:
        {
            IRBuilder builder(autodiffContext.moduleInst);
            auto baseType = inst->getOperand(0);
            SLANG_ASSERT(as<IRDifferentialPairTypeBase>(baseType));
            if (as<IRDifferentialPairType>(baseType) ||
                as<IRDifferentialPairUserCodeType>(baseType))
            {
                return memoize(diffTypeConformanceContext.buildDifferentiablePairWitness(
                    &builder,
                    cast<IRDifferentialPairTypeBase>(baseType),
                    DiffConformanceKind::Value));
            }
            else if (as<IRDifferentialPtrPairType>(baseType))
            {
                return memoize(diffTypeConformanceContext.buildDifferentiablePairWitness(
                    &builder,
                    cast<IRDifferentialPtrPairType>(baseType),
                    DiffConformanceKind::Ptr));
            }
        }
        break;
    case kIROp_SynthesizedBackwardDerivativeWitnessTableFromLegacyBwdDiffFunc:
        {
            SLANG_ASSERT("not supported anymore.. ");
            return memoize(maybeTranslateBackwardDerivativeWitnessFromLegacyBwdDiffFunc(
                &autodiffContext,
                sink,
                cast<IRSynthesizedBackwardDerivativeWitnessTableFromLegacyBwdDiffFunc>(inst)));
        }
        break;
    // Translate special func-types.
    case kIROp_ApplyForBwdFuncType:
    case kIROp_ForwardDiffFuncType:
    case kIROp_FuncResultType:
    case kIROp_BwdCallableFuncType:
    case kIROp_BackwardDiffFuncType:
        {
            DifferentiableTypeConformanceContext ctx(&autodiffContext);
            ctx.setFunc(inst->getParent());
            translationResult = ctx.resolveType(&subBuilder, inst);
        }
        break;
    default:
        break;
    }

    return translationResult;
}

}; // namespace Slang