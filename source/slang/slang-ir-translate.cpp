#include "slang-ir-translate.h"

#include "slang-ir-insts.h"
#include "slang-ir-loop-unroll.h"
#include "slang-ir-peephole.h"
#include "slang-ir-sccp.h"
#include "slang-ir-typeflow-specialize.h"
#include "slang-ir-util.h"
#include "slang-ir.h"

namespace Slang
{


void initializeTranslationDictionary(IRModule* module)
{
    if (!module->getTranslationDict())
    {
        IRBuilder builder(module);
        builder.setInsertInto(module);
        auto dict = cast<IRCompilerDictionary>(
            builder.emitIntrinsicInst(builder.getVoidType(), kIROp_CompilerDictionary, 0, nullptr));
        module->setTranslationDict(dict);

        builder.setInsertInto(dict);
        builder.emitIntrinsicInst(builder.getVoidType(), kIROp_CompilerDictionaryScope, 0, nullptr);
    }
}

void clearTranslationDictionary(IRModule* module)
{
    if (auto translationDict = module->getTranslationDict())
    {
        translationDict->removeAndDeallocate();
    }
    module->setTranslationDict(nullptr);
}

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
            return memoize(maybeTranslateBackwardDerivativeWitness(
                &autodiffContext,
                sink,
                cast<IRSynthesizedBackwardDerivativeWitnessTable>(inst)));
        }
        break;
    case kIROp_MakeIDifferentiableWitness:
        {
            IRBuilder diffPairBuilder(autodiffContext.moduleInst);
            DifferentiableTypeConformanceContext ctx(&autodiffContext);
            auto baseType = inst->getOperand(0);
            SLANG_ASSERT(as<IRDifferentialPairTypeBase>(baseType));
            if (as<IRDifferentialPairType>(baseType))
            {
                return memoize(ctx.buildDifferentiablePairWitness(
                    &diffPairBuilder,
                    cast<IRDifferentialPairTypeBase>(baseType),
                    DiffConformanceKind::Value));
            }
            else if (as<IRDifferentialPtrPairType>(baseType))
            {
                return memoize(ctx.buildDifferentiablePairWitness(
                    &diffPairBuilder,
                    cast<IRDifferentialPtrPairType>(baseType),
                    DiffConformanceKind::Ptr));
            }
        }
        break;
    // Translate special func-types.
    case kIROp_ApplyForBwdFuncType:
    case kIROp_ForwardDiffFuncType:
    case kIROp_BwdCallableFuncType:
    case kIROp_BackwardDiffFuncType:
    case kIROp_RematFuncType:
        {
            DifferentiableTypeConformanceContext ctx(&autodiffContext);
            translationResult = ctx.resolveType(&subBuilder, inst);
        }
        break;
    }

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
    case kIROp_BackwardRemat:
    case kIROp_BackwardDifferentiatePropagate:
    case kIROp_BackwardDiffIntermediateContextType:
    case kIROp_BackwardDiffMinimalContextType:
        {
            auto operand = inst->getOperand(0);

            auto bwdDiffInst =
                subBuilder.emitIntrinsicInst(nullptr, kIROp_BackwardDifferentiate, 1, &operand);

            // Translate the full 5-tuple result.
            auto translatedTuple = maybeTranslateInst(cast<IRBackwardDifferentiate>(bwdDiffInst));
            if (translatedTuple == bwdDiffInst)
                return (bwdDiffInst);

            SLANG_ASSERT(as<IRMakeTuple>(translatedTuple));
            switch (inst->getOp())
            {
            case kIROp_BackwardDifferentiatePrimal:
                return memoize(as<IRMakeTuple>(translatedTuple)->getOperand(0));
            case kIROp_BackwardRemat:
                return memoize(as<IRMakeTuple>(translatedTuple)->getOperand(1));
            case kIROp_BackwardDifferentiatePropagate:
                return memoize(as<IRMakeTuple>(translatedTuple)->getOperand(2));
            case kIROp_BackwardDiffIntermediateContextType:
                return memoize(as<IRMakeTuple>(translatedTuple)->getOperand(3));
            case kIROp_BackwardDiffMinimalContextType:
                return memoize(as<IRMakeTuple>(translatedTuple)->getOperand(4));
            default:
                SLANG_UNEXPECTED("unhandled backward differentiation case");
                break;
            }
        }
        break;
    case kIROp_TrivialBackwardDifferentiatePrimal:
    case kIROp_TrivialBackwardRemat:
    case kIROp_TrivialBackwardDifferentiatePropagate:
    case kIROp_TrivialBackwardDiffIntermediateContextType:
    case kIROp_TrivialBackwardDiffMinimalContextType:
        {
            auto operand = inst->getOperand(0);

            auto bwdDiffInst = subBuilder.emitIntrinsicInst(
                nullptr,
                kIROp_TrivialBackwardDifferentiate,
                1,
                &operand);

            // Translate the full 5-tuple result.
            auto translatedTuple =
                maybeTranslateInst(cast<IRTrivialBackwardDifferentiate>(bwdDiffInst));
            if (translatedTuple == bwdDiffInst)
                return (bwdDiffInst);

            SLANG_ASSERT(as<IRMakeTuple>(translatedTuple));
            switch (inst->getOp())
            {
            case kIROp_TrivialBackwardDifferentiatePrimal:
                return memoize(as<IRMakeTuple>(translatedTuple)->getOperand(0));
            case kIROp_TrivialBackwardRemat:
                return memoize(as<IRMakeTuple>(translatedTuple)->getOperand(1));
            case kIROp_TrivialBackwardDifferentiatePropagate:
                return memoize(as<IRMakeTuple>(translatedTuple)->getOperand(2));
            case kIROp_TrivialBackwardDiffIntermediateContextType:
                return memoize(as<IRMakeTuple>(translatedTuple)->getOperand(3));
            case kIROp_TrivialBackwardDiffMinimalContextType:
                return memoize(as<IRMakeTuple>(translatedTuple)->getOperand(4));
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
    case kIROp_BackwardRematFromLegacyBwdDiffFunc:
    case kIROp_BackwardPropagateFromLegacyBwdDiffFunc:
    case kIROp_BackwardMinimalContextFromLegacyBwdDiffFunc:
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

            // Translate the full 5-tuple result.
            auto translatedTuple =
                maybeTranslateInst(cast<IRBackwardFromLegacyBwdDiffFunc>(legacyToNewBwdDiffInst));
            if (translatedTuple == legacyToNewBwdDiffInst)
                return (legacyToNewBwdDiffInst);

            SLANG_ASSERT(as<IRMakeTuple>(translatedTuple));
            switch (inst->getOp())
            {
            case kIROp_BackwardPrimalFromLegacyBwdDiffFunc:
                return memoize(as<IRMakeTuple>(translatedTuple)->getOperand(0));
            case kIROp_BackwardRematFromLegacyBwdDiffFunc:
                return memoize(as<IRMakeTuple>(translatedTuple)->getOperand(1));
            case kIROp_BackwardPropagateFromLegacyBwdDiffFunc:
                return memoize(as<IRMakeTuple>(translatedTuple)->getOperand(2));
            case kIROp_BackwardContextFromLegacyBwdDiffFunc:
                return memoize(as<IRMakeTuple>(translatedTuple)->getOperand(3));
            case kIROp_BackwardMinimalContextFromLegacyBwdDiffFunc:
                return memoize(as<IRMakeTuple>(translatedTuple)->getOperand(4));
            default:
                SLANG_UNEXPECTED("unhandled legacy backward differentiation case");
                break;
            }
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
    auto witnessTable = as<IRWitnessTable>(lookupInst->getWitnessTable());
    if (!witnessTable)
        return lookupInst;

    // Because we have a concrete witness table, we can
    // use it to look up the IR value that satisfies
    // the given interface requirement.
    //
    auto requirementKey = lookupInst->getRequirementKey();
    auto satisfyingVal = findWitnessTableEntry(witnessTable, requirementKey);

    lookupInst->replaceUsesWith(satisfyingVal);
    lookupInst->removeAndDeallocate();

    return satisfyingVal;
}

// Resolve any specialization and translations.
IRInst* _resolveInstRec(TranslationContext* ctx, IRInst* inst)
{
    if (!inst)
        return nullptr;

    // Don't attempt to resolve insts that are potentially recursive.
    if (as<IRInterfaceType>(inst) || as<IRWitnessTable>(inst))
    {
        return inst;
    }

    // IRParam is a function-scope value, not a type or specialization construct.
    // It can appear as an operand of ExtractExistentialType when interface-typed
    // entry-point params remain at function scope (e.g. CUDA compute).
    if (as<IRParam>(inst))
        return inst;

    IRBuilder builder(ctx->getModule());
    IRWeakUse* instRef = builder.getWeakUse(inst);

    // Make sure all operands are resolved.
    for (UInt i = 0; i < inst->getOperandCount(); ++i)
    {
        auto operand = inst->getOperand(i);
        ctx->resolveInst(operand);
    }

    // Extract effective inst post-resolution. (the inst may have changed).
    IRInst* instWithCanonicalOperands = instRef->getOperand(0);

    if (isEvaluableOpCode(instWithCanonicalOperands->getOp()))
    {
        if (auto constFoldedResult = tryConstantFoldInst(
                ctx->getModule(),
                ctx->getTargetProgram(),
                instWithCanonicalOperands))
        {
            instWithCanonicalOperands->replaceUsesWith(constFoldedResult);
            return constFoldedResult;
        }
        else
        {
            SLANG_UNEXPECTED(
                "Something went wrong.. a global inst with evaluable opcode should have been "
                "constant folded");
        }
    }
    else
    {
        switch (instWithCanonicalOperands->getOp())
        {
        case kIROp_SizeOf:
        case kIROp_AlignOf:
        case kIROp_GetArrayLength:
            if (tryReplaceInstUsesWithSimplifiedValue(
                    ctx->getTargetProgram(),
                    ctx->getModule(),
                    instWithCanonicalOperands))
            {
                return instRef->getOperand(0);
            }
            break;
        default:
            break;
        }
    }

    // At this point, we've resolved anything that can be translated & not in the global scope (i.e.
    // things like arithmetic operations)
    //
    // If the instruction is not at module scope, it cannot be resolved further.
    // This can happen when a generic is specialized with an existential/interface type,
    // producing function-local instructions that reach this resolution logic.
    // Return the instruction as-is and let later passes handle the diagnostic.
    //
    if (!as<IRModuleInst>(instWithCanonicalOperands->getParent()))
        return instWithCanonicalOperands;

    // TODO: Group these.
    if (as<IRTranslateBase>(instWithCanonicalOperands) ||
        as<IRTranslatedTypeBase>(instWithCanonicalOperands) ||
        as<IRForwardDiffFuncType>(instWithCanonicalOperands) ||
        as<IRBwdCallableFuncType>(instWithCanonicalOperands) ||
        as<IRApplyForBwdFuncType>(instWithCanonicalOperands) ||
        as<IRBackwardDiffFuncType>(instWithCanonicalOperands))
    {
        auto translateInst = ctx->maybeTranslateInst(instWithCanonicalOperands);
        instWithCanonicalOperands->replaceUsesWith(translateInst);
        return translateInst;
    }

    // Assume at this point that we have a specializable inst with resolved operands.
    auto entry = builder.fetchCompilerDictionaryEntry(
        ctx->getModule()->getTranslationDict(),
        instWithCanonicalOperands);

    if (auto specializedInst = entry->getValue())
    {
        instWithCanonicalOperands->replaceUsesWith(specializedInst);
        return specializedInst;
    }

    auto memoize = [&](IRInst* resultInst)
    {
        builder.setCompilerDictionaryEntryValue(entry, resultInst);
        return resultInst;
    };

    switch (instWithCanonicalOperands->getOp())
    {
    case kIROp_Specialize:
        {
            if (!isSetSpecializedGeneric(instWithCanonicalOperands))
            {
                auto specInst = cast<IRSpecialize>(instWithCanonicalOperands);
                auto specResult = specializeGeneric(ctx->getSpecializationContext(), specInst);

                if (specResult && as<IRGlobalValueWithCode>(specResult))
                {
                    // If we ended up with something that has code,
                    // specialization may have opened up some simplification opportunities.
                    //

                    applySparseConditionalConstantPropagation(
                        specResult,
                        ctx->getTargetProgram(),
                        ctx->getSink(),
                        ctx);

                    if (!unrollLoopsInFunc(
                            ctx->getTargetProgram(),
                            ctx->getModule(),
                            as<IRGlobalValueWithCode>(specResult),
                            ctx->getSink()))
                        return nullptr;
                }

                if (specResult && specResult != specInst)
                    specInst->replaceUsesWith(specResult);

                // No need to memoize since specializeGeneric will already have memoized this.
                return specResult;
            }
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
    return nullptr;
}

}; // namespace Slang
