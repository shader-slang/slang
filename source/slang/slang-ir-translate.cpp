#include "slang-ir-translate.h"

#include "slang-ir-insts.h"
#include "slang-ir-sccp.h"
#include "slang-ir-typeflow-specialize.h"
#include "slang-ir-util.h"
#include "slang-ir.h"

namespace Slang
{

IRInst* TranslationContext::maybeTranslateInst(IRInst* inst)
{
    IRBuilder builder(irModule);

    if (auto existingTranslation =
            builder.tryLookupCompilerDictionaryValue(irModule->getTranslationDict(), inst))
    {
        return existingTranslation;
    }

    auto memoize = [&](IRInst* resultInst)
    {
        builder.addCompilerDictionaryEntry(irModule->getTranslationDict(), inst, resultInst);
        return resultInst;
    };

    IRInst* translationResult = nullptr;
    IRBuilder subBuilder(inst->getModule());
    subBuilder.setInsertBefore(inst);
    switch (inst->getOp())
    {
    case kIROp_BackwardDifferentiate:
        {
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
                return (bwdDiffInst);

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
                return (bwdDiffInst);

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
                return (legacyToNewBwdDiffInst);

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
            DifferentiableTypeConformanceContext ctx(&autodiffContext);
            auto baseType = inst->getOperand(0);
            SLANG_ASSERT(as<IRDifferentialPairTypeBase>(baseType));
            if (as<IRDifferentialPairType>(baseType) ||
                as<IRDifferentialPairUserCodeType>(baseType))
            {
                return memoize(ctx.buildDifferentiablePairWitness(
                    &builder,
                    cast<IRDifferentialPairTypeBase>(baseType),
                    DiffConformanceKind::Value));
            }
            else if (as<IRDifferentialPtrPairType>(baseType))
            {
                return memoize(ctx.buildDifferentiablePairWitness(
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

static IRInst* specializeWitnessLookup(IRLookupWitnessMethod* lookupInst)
{
    // We can only specialize in the case where the lookup
    // is being done on a concrete witness table, and not
    // the result of a `specialize` instruction or other
    // operation that will yield such a table.
    //
    auto witnessTable = cast<IRWitnessTable>(lookupInst->getWitnessTable());

    // Because we have a concrete witness table, we can
    // use it to look up the IR value that satisfies
    // the given interface requirement.
    //
    auto requirementKey = lookupInst->getRequirementKey();
    auto satisfyingVal = findWitnessTableEntry(witnessTable, requirementKey);

    return satisfyingVal;
}

// TODO: DEDUPLICATE with constant folding code?
static bool isEvaluableOpCode(IROp op)
{
    switch (op)
    {
    case kIROp_IntLit:
    case kIROp_BoolLit:
    case kIROp_FloatLit:
    case kIROp_StringLit:
    case kIROp_Add:
    case kIROp_Sub:
    case kIROp_Mul:
    case kIROp_Div:
    case kIROp_Neg:
    case kIROp_Not:
    case kIROp_Eql:
    case kIROp_Neq:
    case kIROp_Leq:
    case kIROp_Geq:
    case kIROp_Less:
    case kIROp_IRem:
    case kIROp_FRem:
    case kIROp_Greater:
    case kIROp_Lsh:
    case kIROp_Rsh:
    case kIROp_BitAnd:
    case kIROp_BitOr:
    case kIROp_BitXor:
    case kIROp_BitNot:
    case kIROp_BitCast:
    case kIROp_CastIntToFloat:
    case kIROp_CastFloatToInt:
    case kIROp_IntCast:
    case kIROp_FloatCast:
    case kIROp_Select:
        return true;
    default:
        return false;
    }
}

// Resolve any specialization and translations.
IRInst* _resolveInstRec(TranslationContext* ctx, IRInst* inst)
{
    if (!inst)
        return nullptr;

    // Don't attempt to resolve insts that are potentially recursive.
    if (as<IRInterfaceType>(inst))
    {
        return inst;
    }

    SLANG_ASSERT(as<IRModuleInst>(inst->getParent()));

    if (isEvaluableOpCode(inst->getOp()))
    {
        if (auto constFoldedResult = tryConstantFoldInst(ctx->getModule(), inst))
        {
            inst->replaceUsesWith(constFoldedResult);
            return constFoldedResult;
        }
        else
        {
            SLANG_UNEXPECTED(
                "Something went wrong.. a global inst with evaluable opcode should have been "
                "constant folded");
        }
    }

    // Otherwise we'll fall back to recreating the instruction

    List<IRInst*> operands;
    bool changed = false;

    IRBuilder builder(ctx->getModule());
    IRWeakUse* instRef = builder.getWeakUse(inst);

    // Make sure all operands are resolved.
    for (UInt i = 0; i < inst->getOperandCount(); ++i)
    {
        auto operand = inst->getOperand(i);
        auto resolvedOperand = ctx->resolveInst(operand);
        if (resolvedOperand != operand)
            changed = true;
        // operands.add(resolvedOperand);
    }

    // If any operand changed, we need to recreate the instruction.
    /*
    IRInst* instWithCanonicalOperands = inst;
    if (changed)
    {
        IRBuilder builder(ctx->getModule());
        builder.setInsertBefore(inst);
        instWithCanonicalOperands = builder.emitIntrinsicInst(
            inst->getDataType(),
            inst->getOp(),
            operands.getCount(),
            operands.getBuffer());
    }*/

    // Extract effective inst post-resolution. (the inst may have changed).
    IRInst* instWithCanonicalOperands = instRef->getOperand(0);

    if (as<IRTranslateBase>(instWithCanonicalOperands) ||
        as<IRTranslatedTypeBase>(instWithCanonicalOperands))
    {
        auto translateInst = ctx->maybeTranslateInst(instWithCanonicalOperands);
        instWithCanonicalOperands->replaceUsesWith(translateInst);
        return translateInst;
    }

    // Assume at this point that we have a specializable inst with resolved operands.
    if (auto specializedInst = builder.tryLookupCompilerDictionaryValue(
            ctx->getModule()->getTranslationDict(),
            instWithCanonicalOperands))
    {
        instWithCanonicalOperands->replaceUsesWith(specializedInst);
        return specializedInst;
    }

    auto memoize = [&](IRInst* resultInst)
    {
        builder.addCompilerDictionaryEntry(
            ctx->getModule()->getTranslationDict(),
            instWithCanonicalOperands,
            resultInst);
        return resultInst;
    };

    switch (instWithCanonicalOperands->getOp())
    {
    case kIROp_Specialize:
        {
            if (!isSetSpecializedGeneric(instWithCanonicalOperands))
                return memoize(specializeGeneric(cast<IRSpecialize>(instWithCanonicalOperands)));
            break;
        }
    case kIROp_LookupWitnessMethod:
        return memoize(
            specializeWitnessLookup(cast<IRLookupWitnessMethod>(instWithCanonicalOperands)));
    }

    return instWithCanonicalOperands;
}

IRInst* TranslationContext::resolveInst(IRInst* inst)
{
    IRBuilder builder(irModule);
    while (auto resolvedInst = _resolveInstRec(this, inst))
    {
        if (resolvedInst == inst)
        {
            return resolvedInst;
        }

        inst = resolvedInst;
    }
}

}; // namespace Slang