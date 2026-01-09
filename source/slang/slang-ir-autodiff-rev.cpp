#include "slang-ir-autodiff-rev.h"

#include "slang-ir-autodiff-cfg-norm.h"
#include "slang-ir-autodiff-fwd.h"
#include "slang-ir-clone.h"
#include "slang-ir-dce.h"
#include "slang-ir-dominators.h"
#include "slang-ir-eliminate-multilevel-break.h"
#include "slang-ir-init-local-var.h"
#include "slang-ir-inline.h"
#include "slang-ir-inst-pass-base.h"
#include "slang-ir-loop-unroll.h"
#include "slang-ir-propagate-func-properties.h"
#include "slang-ir-redundancy-removal.h"
#include "slang-ir-single-return.h"
#include "slang-ir-ssa-simplification.h"
#include "slang-ir-util.h"

namespace Slang
{

// Returns "dp<var-name>" to use as a name hint for parameters.
// If no primal name is available, returns a blank string.
//
String BackwardDiffTranscriberBase::makeDiffPairName(IRInst* origVar)
{
    if (auto namehintDecoration = origVar->findDecoration<IRNameHintDecoration>())
    {
        return ("dp" + String(namehintDecoration->getName()));
    }

    return String("");
}

// Puts parameters into their own block.
void BackwardDiffTranscriberBase::makeParameterBlock(IRBuilder* inBuilder, IRFunc* func)
{
    IRBuilder builder = *inBuilder;

    auto firstBlock = func->getFirstBlock();
    IRParam* param = func->getFirstParam();

    builder.setInsertBefore(firstBlock);

    // Note: It looks like emitBlock() doesn't use the current
    // builder position, so we're going to manually move the new block
    // to before the existing block.
    auto paramBlock = builder.emitBlock();
    builder.markInstAsMixedDifferential(paramBlock);
    paramBlock->insertBefore(firstBlock);
    builder.setInsertInto(paramBlock);

    while (param)
    {
        IRParam* nextParam = param->getNextParam();

        // Move inst into the new parameter block.
        param->insertAtEnd(paramBlock);

        param = nextParam;
    }

    // Replace this block as the first block.
    firstBlock->replaceUsesWith(paramBlock);

    // Add terminator inst.
    builder.emitBranch(firstBlock);
}

SlangResult BackwardDiffTranscriberBase::prepareFuncForBackwardDiff(IRFunc* func)
{
    removeLinkageDecorations(func);

    performPreAutoDiffForceInlining(func);

    DifferentiableTypeConformanceContext diffTypeContext(autoDiffSharedContext);

    auto returnCount = getReturnCount(func);
    if (returnCount > 1)
    {
        convertFuncToSingleReturnForm(func->getModule(), func);
    }
    else if (returnCount == 0)
    {
        // The function is ill-formed and never returns (such as having an infinite loop),
        // we can't possibly reverse-differentiate such functions, so we will diagnose it here.
        getSink()->diagnose(func->sourceLoc, Diagnostics::functionNeverReturnsFatal, func);
    }

    eliminateContinueBlocksInFunc(func->getModule(), func);

    eliminateMultiLevelBreakForFunc(func->getModule(), func);

    IRCFGNormalizationPass cfgPass = {this->getSink()};
    normalizeCFG(autoDiffSharedContext->moduleInst->getModule(), func, cfgPass);

    return SLANG_OK;
}

// Create a copy of originalFunc's forward derivative in the same generic context (if any) of
// `diffPropagateFunc`.
IRFunc* BackwardDiffTranscriberBase::generateNewForwardDerivativeForFunc(
    IRBuilder* builder,
    IRFunc* originalFunc,
    IRFunc* diffPropagateFunc)
{
    // Make a clone of original func so we won't modify the original.
    IRCloneEnv originalCloneEnv;
    auto clonedFunc = cloneInst(&originalCloneEnv, builder, originalFunc);
    auto primalFunc = as<IRFunc>(clonedFunc);

    // Strip any existing derivative decorations off the clone.
    stripDerivativeDecorations(primalFunc);
    eliminateDeadCode(primalFunc);

    // Perform required transformations and simplifications on the original func to make it
    // reversible.
    if (SLANG_FAILED(prepareFuncForBackwardDiff(primalFunc)))
        return diffPropagateFunc;

    ForwardDiffTranscriber fwdTranscriber(autoDiffSharedContext, sink);
    fwdTranscriber.enableReverseModeCompatibility();
    IRFunc* fwdDiffFunc =
        as<IRFunc>(fwdTranscriber.transcribeFuncHeader(builder, primalFunc).differential);
    SLANG_ASSERT(fwdDiffFunc);
    fwdTranscriber.transcribeFunc(builder, primalFunc, fwdDiffFunc);
    fwdDiffFunc->sourceLoc = primalFunc->sourceLoc;

    // Remove the clone of original func.
    SLANG_ASSERT(primalFunc->hasUses() == false);
    primalFunc->removeAndDeallocate();

    // Remove redundant loads since they interfere with transposition logic.
    eliminateRedundantLoadStore(fwdDiffFunc);

    return fwdDiffFunc;
}

IRFunc* BackwardDiffTranscriberBase::generateTrivialForwardDerivativeForFunc(
    IRBuilder* builder,
    IRFunc* originalFunc,
    IRFunc* diffPropagateFunc)
{
    IRInst* operand = originalFunc;
    // Generate a `OpTrivialForwardDifferentiate' inst and translate it.
    IRTrivialForwardDifferentiate* trivialFwdDiffInst =
        cast<IRTrivialForwardDifferentiate>(builder->emitIntrinsicInst(
            diffPropagateFunc->getDataType(),
            kIROp_TrivialForwardDifferentiate,
            1,
            &operand));
    return cast<IRFunc>(maybeTranslateTrivialForwardDerivative(
        autoDiffSharedContext,
        getSink(),
        trivialFwdDiffInst));
}

static void generateName(IRBuilder* builder, IRInst* srcInst, IRInst* dstInst, const char* prefix)
{
    if (auto nameHint = srcInst->findDecoration<IRNameHintDecoration>())
    {
        String name = nameHint->getName();
        name = prefix + name;
        builder->addNameHintDecoration(dstInst, name.getUnownedSlice());
    }
}

static IRInst* maybeHoist(IRBuilder& builder, IRInst* inst)
{
    IRInst* specializedVal = nullptr;
    auto hoistResult = hoistValueFromGeneric(builder, inst, specializedVal, true);
    return hoistResult; //(as<IRGeneric>(hoistResult)) ? getGenericReturnVal(hoistResult) :
                        // hoistResult;
}

static IRInst* maybeHoistAndSpecialize(IRBuilder& builder, IRInst* inst)
{
    IRInst* specializedVal = nullptr;
    auto hoistResult = hoistValueFromGeneric(builder, inst, specializedVal, true);
    return (specializedVal) ? specializedVal : hoistResult;
}

// Transcribe a function definition (AD 2.0)
void BackwardDiffTranscriberBase::_transcribeFuncImpl(
    IRBuilder* builder,
    IRFunc* targetFunc,
    IRInst*& applyFuncInst,
    IRInst*& propagateFuncInst,
    IRInst*& contextGetValFuncInst,
    IRInst*& contextTypeInst,
    bool isTrivial)
{

    // --------------------------------------------------------------------------
    // Create IRFunc* for propagate function &
    // create the IRFuncType for it.
    //
    builder->setInsertAfter(targetFunc);
    auto propagateFunc = builder->createFunc();


    IRBuilder tempBuilder = *builder;
    tempBuilder.setInsertBefore(propagateFunc);

    auto fwdDiffFunc =
        (!isTrivial)
            ? generateNewForwardDerivativeForFunc(&tempBuilder, targetFunc, propagateFunc)
            : generateTrivialForwardDerivativeForFunc(&tempBuilder, targetFunc, propagateFunc);
    if (!fwdDiffFunc)
        return;

    // Split first block into a paramter block.
    this->makeParameterBlock(&tempBuilder, as<IRFunc>(fwdDiffFunc));

    diffUnzipPass->unzipDiffInsts(fwdDiffFunc);
    IRFunc* unzippedFwdDiffFunc = fwdDiffFunc;

    // Move blocks from `unzippedFwdDiffFunc` to the `diffPropagateFunc` shell.
    builder->setInsertInto(propagateFunc->getParent());
    {
        List<IRBlock*> workList;
        for (auto block = unzippedFwdDiffFunc->getFirstBlock(); block;
             block = block->getNextBlock())
            workList.add(block);

        for (auto block : workList)
            block->insertAtEnd(propagateFunc);
    }

    builder->setInsertInto(propagateFunc);

    // Transpose differential blocks from unzippedFwdDiffFunc into diffFunc (with dOutParameter)
    // representing the derivative of the return value.
    diffTransposePass->transposeDiffBlocksInFunc(propagateFunc, {});

    // Apply checkpointing policy to legalize cross-scope uses of primal values
    // using either recompute or store strategies.
    auto primalsInfo = applyCheckpointPolicy(propagateFunc);

    // Extracts the primal computations into its own func, turn all accesses to stored primal insts
    // into explicit intermediate data structure reads and writes.
    IRInst* intermediateType = nullptr;
    IRFunc* getValFunc = nullptr;
    auto applyFunc = diffUnzipPass->extractPrimalFunc(
        propagateFunc,
        targetFunc,
        primalsInfo,
        intermediateType,
        getValFunc);

    // At this point the unzipped func is just an empty shell
    // and we can simply remove it.
    unzippedFwdDiffFunc->removeAndDeallocate();

    // Copy over checkpoint preference hints.
    {
        auto diffPrimalFunc = getResolvedInstForDecorations(applyFunc, true);
        auto checkpointHint = targetFunc->findDecoration<IRCheckpointHintDecoration>();
        if (checkpointHint)
            builder->addDecoration(diffPrimalFunc, checkpointHint->getOp());
    }

    // ------------------------------------------------------------
    // Fill in the propagate function's type.
    List<IRType*> propagateParamTypes;
    IRType* propagateResultType;

    propagateParamTypes.add((IRType*)intermediateType);

    for (UInt i = 0; i < targetFunc->getParamCount(); i++)
    {
        const auto& [direction, paramType] =
            splitParameterDirectionAndType(targetFunc->getParamType(i));
        auto diffValueParamType =
            (IRType*)differentiableTypeConformanceContext.tryGetAssociationOfKind(
                paramType,
                ValAssociationKind::DifferentialType);

        if (diffValueParamType)
            propagateParamTypes.add(
                fromDirectionAndType(builder, transposeDirection(direction), diffValueParamType));
        else
            propagateParamTypes.add(builder->getVoidType());
    }

    auto resultType = targetFunc->getResultType();
    auto diffResultType = (IRType*)differentiableTypeConformanceContext.tryGetAssociationOfKind(
        resultType,
        ValAssociationKind::DifferentialType);
    if (diffResultType)
    {
        propagateResultType =
            fromDirectionAndType(builder, ParameterDirectionInfo::Kind::In, diffResultType);
    }
    else
    {
        propagateResultType = builder->getVoidType();
    }

    if (propagateResultType->getOp() != kIROp_VoidType)
    {
        // If the result type is not void, we need to add it as the last parameter.
        propagateParamTypes.add(propagateResultType);
    }

    auto propagateFuncType = builder->getFuncType(propagateParamTypes, builder->getVoidType());
    propagateFunc->setFullType(propagateFuncType);

    // --------------------------------------------------------------------------

    initializeLocalVariables(builder->getModule(), applyFunc);
    initializeLocalVariables(builder->getModule(), propagateFunc);

    // Clean up block labels & other temp decorations.
    stripTempDecorations(propagateFunc);
    stripTempDecorations(applyFunc);
    stripTempDecorations(getValFunc);

    // Make sure blocks are in control-flow order.
    sortBlocksInFunc(propagateFunc);
    sortBlocksInFunc(targetFunc);

    generateName(builder, targetFunc, applyFunc, "s_apply_");
    generateName(builder, targetFunc, propagateFunc, "s_bwdProp_");
    generateName(builder, targetFunc, getValFunc, "s_getVal_");
    generateName(builder, targetFunc, intermediateType, "s_bwdCallableCtx_");

    copyDebugInfo(targetFunc, applyFunc);
    copyDebugInfo(targetFunc, propagateFunc);
    copyDebugInfo(targetFunc, getValFunc);

    propagatePropertiesForSingleFunc(builder->getModule(), propagateFunc);
    propagatePropertiesForSingleFunc(builder->getModule(), applyFunc);
    propagatePropertiesForSingleFunc(builder->getModule(), getValFunc);

    IRBuilder subBuilder = *builder;

    //
    // Output the 4-tuple result of the translation (and hoist values out of any generic contexts).
    //

    // It's important to hoist the context type out *first* because the other funcs may depend on
    // it.
    //
    contextTypeInst = maybeHoist(subBuilder, intermediateType);

    propagateFuncInst = maybeHoist(subBuilder, propagateFunc);
    applyFuncInst = maybeHoist(subBuilder, applyFunc);
    contextGetValFuncInst = maybeHoist(subBuilder, getValFunc);
}

ParameterBlockTransposeInfo BackwardDiffTranscriberBase::splitAndTransposeParameterBlock(
    IRBuilder* builder,
    IRFunc* diffFunc,
    SourceLoc primalLoc,
    bool isResultDifferentiable)
{
    // This method splits transposes the all the parameters for both the primal and propagate
    // computation. At the end of this method, the parameter block will contain a combination of
    // parameters for both the to-be-primal function and to-be-propagate function. We use
    // ParameterBlockTransposeInfo::primalFuncParams and
    // ParameterBlockTransposeInfo::propagateFuncParams to track which parameters are dedicated to
    // the future primal or propagate func. A later step will then split the parameters out to each
    // new function.

    ParameterBlockTransposeInfo result;

    // First, we initialize the IR builders and locate the import code insertion points that will
    // be used for the rest of this method.

    IRBlock* fwdDiffParameterBlock = diffFunc->getFirstBlock();

    // Find the 'next' block using the terminator inst of the parameter block.
    auto fwdParamBlockBranch = as<IRUnconditionalBranch>(fwdDiffParameterBlock->getTerminator());
    // We create a new block after parameter block to hold insts that translates from transposed
    // parameters into something that the rest of the function can use.
    IRBuilder::insertBlockAlongEdge(diffFunc->getModule(), IREdge(&fwdParamBlockBranch->block));
    auto paramPreludeBlock = fwdParamBlockBranch->getTargetBlock();

    auto nextBlockBuilder = *builder;
    nextBlockBuilder.setInsertBefore(paramPreludeBlock->getFirstOrdinaryInst());

    SourceLoc returnLoc;
    IRBlock* firstDiffBlock = nullptr;
    for (auto block : diffFunc->getBlocks())
    {
        if (isDifferentialInst(block))
        {
            firstDiffBlock = block;
            break;
        }

        auto terminator = block->getTerminator();
        if (as<IRReturn>(terminator))
        {
            returnLoc = terminator->sourceLoc;
            break;
        }
    }

    SLANG_RELEASE_ASSERT(firstDiffBlock);

    auto diffBuilder = *builder;
    diffBuilder.setInsertBefore(firstDiffBlock->getFirstOrdinaryInst());

    builder->setInsertBefore(fwdParamBlockBranch);

    // Collect all the original parameters.
    List<IRParam*> fwdParams;
    for (auto param : diffFunc->getParams())
        fwdParams.add(param);

    // Maintain a set for insts pending removal.
    OrderedHashSet<IRInst*> instsToRemove;

    // Now we begin the actual processing.
    // The first step is to transcribe all the existing parameters from the original function.
    // There are many cases to handle, including different combinations of parameter directions and
    // whether or not the parameter is differentiable.
    // To normalize the process for all these cases, we determine the following actions for each
    // parameter:
    // 1. Should this original parameter be translated to a parameter in the primal func and the
    // propagate func?
    //    if so, we emit a param inst representing the final parameter for that func. If the
    //    parameter should be mapped to both the primal func and the propagate func, we will emit
    //    two separate params with their final type.
    // 2. If this parameter has a corresponding primal func parameter, we replace all uses of the
    // original
    //    parameter in the primal computation code to the new primal parameter. If any
    //    initialization logic is needed to convert the type of the new primal parameter to what the
    //    code was expecting, we insert that code in the first block.
    // 3. If this parameter has a correponding propagate func parameter, we replace all uses of the
    // original parameter
    //    in the diff computation code to the new propagate parameter. We insert necessary
    //    initialization diff block or the first block depending on whether we want that logic go
    //    through the transposition pass. We may need to replace the uses to different
    //    values/variables depending on whether that use is a read or write.
    // 4. If the parameter has both corresponding primal and propagate parameters, we also need to
    // consider
    //    how the future propagate function access the primal parameter. We will insert necessary
    //    preparation code that constructs temp vars or values to replace the primal parameter after
    //    we remove it from the propagate func.
    // Base on above discussion, we need to compute the following values for each parameter:
    // - diffRefReplacement. What should all read(load) references to this parameter from
    // differential code be replaced to.
    // - diffRefWriteReplacement. What should all write references to this parameter from
    // differential code be replaced to.
    // - primalRefReplacement. What should all references to this parameter from primal code be
    // replaced to.
    // - mapPrimalSpecificParamToReplacementInPropFunc[param]. What should all references to this
    // parameter
    //      from the primal compuation logic in the future propagate function be replaced to.

    auto ctxParam = builder->emitParam(as<IRFuncType>(diffFunc->getDataType())->getParamType(0));
    builder->addNameHintDecoration(ctxParam, UnownedStringSlice("_s_diff_ctx"));
    builder->addDecoration(ctxParam, kIROp_PrimalContextDecoration);
    result.propagateFuncParams.add(ctxParam);

    diffFunc->sourceLoc = primalLoc;
    ctxParam->sourceLoc = primalLoc;

    for (auto fwdParam : fwdParams)
    {
        IRBuilderSourceLocRAII sourceLocationScope(builder, fwdParam->sourceLoc);

        // Define the replacement insts that we are going to fill in for each case.
        IRInst* diffRefReplacement = nullptr;
        IRInst* primalRefReplacement = nullptr;
        IRInst* diffWriteRefReplacement = nullptr;

        // Common logic that computes all the important types we care about.
        IRDifferentialPairType* diffPairType = as<IRDifferentialPairType>(fwdParam->getDataType());
        auto inoutType = as<IRBorrowInOutParamType>(fwdParam->getDataType());
        auto outType = as<IROutParamType>(fwdParam->getDataType());
        if (inoutType)
            diffPairType = as<IRDifferentialPairType>(inoutType->getValueType());
        else if (outType)
            diffPairType = as<IRDifferentialPairType>(outType->getValueType());
        IRType* primalType = nullptr;
        IRType* diffType = nullptr;
        if (diffPairType)
        {
            primalType = diffPairType->getValueType();
            diffType = (IRType*)differentiableTypeConformanceContext.getDiffTypeFromPairType(
                builder,
                diffPairType);
        }
        else
        {
            primalType = fwdParam->getDataType();

            if (auto outType = as<IROutParamType>(primalType))
                primalType = outType->getValueType();
            else if (auto inoutType = as<IRBorrowInOutParamType>(primalType))
                primalType = inoutType->getValueType();
        }

        // AD 2.0 logic (significantly simplified)
        // If the parameter is a relevant differential pair, we
        // put the primal component in the primal function and the diff component
        // in the propagate function.
        // If it's not relevant, then we replace it with a none-type parameter.
        //
        switch (fwdParam->getDataType()->getOp())
        {
        case kIROp_OutParamType:
            // Out.
            if (diffType)
            {
                diffWriteRefReplacement = builder->emitParam(diffType); // In diff.
                markDiffTypeInst(builder, diffWriteRefReplacement, primalType);

                result.propagateFuncParams.add(diffWriteRefReplacement);
                copyNameHintAndDebugDecorations(diffWriteRefReplacement, fwdParam);
                diffRefReplacement = nullptr;
            }
            else
            {
                // NoneType parameter.
                result.propagateFuncParams.add(builder->emitParam(builder->getVoidType()));
            }

            primalRefReplacement = builder->emitParam( // Out primal.
                builder->getOutParamType(primalType));
            result.primalFuncParams.add(primalRefReplacement);
            copyNameHintAndDebugDecorations(primalRefReplacement, fwdParam);

            break;

        case kIROp_BorrowInOutParamType:
            // In Out.
            if (diffType)
            {
                auto diffParam =
                    builder->emitParam(builder->getBorrowInOutParamType(diffType)); // InOut diff.
                markDiffTypeInst(builder, diffParam, primalType);

                result.propagateFuncParams.add(diffParam);
                copyNameHintAndDebugDecorations(diffParam, fwdParam);

                diffRefReplacement = diffParam;
                diffWriteRefReplacement = diffParam;
            }
            else
            {
                // NoneType parameter.
                result.propagateFuncParams.add(builder->emitParam(builder->getVoidType()));
            }

            primalRefReplacement =
                builder->emitParam(builder->getBorrowInOutParamType(primalType)); // InOut primal.
            result.primalFuncParams.add(primalRefReplacement);
            break;

        case kIROp_RefParamType:
        case kIROp_BorrowInParamType:
            SLANG_UNEXPECTED("Unexpected ref/constref type in backward diff transcriber");
            break;

        default:
            // In.
            if (diffPairType)
            {
                auto diffParam =
                    builder->emitParam(builder->getOutParamType(diffType)); // Out diff.
                markDiffTypeInst(builder, diffParam, primalType);

                result.propagateFuncParams.add(diffParam);

                diffWriteRefReplacement = nullptr;
                diffRefReplacement = diffParam;
            }
            else
            {
                // NoneType parameter.
                result.propagateFuncParams.add(builder->emitParam(builder->getVoidType()));
            }

            primalRefReplacement = builder->emitParam(primalType); // Out primal.
            result.primalFuncParams.add(primalRefReplacement);
            break;
        }

        // We have emitted all the new parameters and computed the replacements for the original
        // parameter. Now we perform that replacement.
        List<IRUse*> uses;
        for (auto use = fwdParam->firstUse; use; use = use->nextUse)
            uses.add(use);

        for (auto use : uses)
        {
            if (auto primalRef = as<IRPrimalParamRef>(use->getUser()))
            {
                SLANG_RELEASE_ASSERT(primalRefReplacement);
                primalRef->replaceUsesWith(primalRefReplacement);
                instsToRemove.add(primalRef);
            }
            else if (auto getPrimal = as<IRDifferentialPairGetPrimal>(use->getUser()))
            {
                SLANG_RELEASE_ASSERT(primalRefReplacement);
                getPrimal->replaceUsesWith(primalRefReplacement);
                instsToRemove.add(getPrimal);
            }
            else if (auto propagateRef = as<IRDiffParamRef>(use->getUser()))
            {
                result.transposedInstMap[propagateRef] = diffRefReplacement;
            }
            else if (auto getDiff = as<IRDifferentialPairGetDifferential>(use->getUser()))
            {
                SLANG_RELEASE_ASSERT(diffRefReplacement);

                result.transposedInstMap[getDiff] = diffRefReplacement;
            }
            else
            {
                // If the user is something else, it'd better be a non relevant parameter.
                if (diffRefReplacement || diffWriteRefReplacement)
                    SLANG_UNEXPECTED("unknown use of parameter.");
                use->set(primalRefReplacement);
            }
        }
    }

    // Actually remove all the insts that we decided to remove in the process.
    for (auto inst : instsToRemove)
    {
        inst->removeAndDeallocate();
    }


    // The next step is to insert new parameters that is not related to any existing parameters.
    //
    // If the return type of the original function is differentiable,
    // add a parameter for 'derivative of the output' (d_out).
    // The type is the second last parameter type of the function.
    //
    auto paramCount = as<IRFuncType>(diffFunc->getDataType())->getParamCount();
    IRParam* dOutParam = nullptr;
    if (isResultDifferentiable)
    {
        auto dOutParamType = as<IRFuncType>(diffFunc->getDataType())->getParamType(paramCount - 1);

        SLANG_ASSERT(dOutParamType);

        dOutParam = builder->emitParam(dOutParamType);
        dOutParam->sourceLoc = returnLoc;
        builder->addNameHintDecoration(dOutParam, UnownedStringSlice("_s_dOut"));
        result.propagateFuncParams.add(dOutParam);
    }

    result.dOutParam = dOutParam;

    return result;
}


LegacyBackwardDiffTranslationFuncContext::Result LegacyBackwardDiffTranslationFuncContext::
    translate(IRBuilder* builder)
{
    // We just need to call the applyBwdFunc() with all the primal parts of the parameters
    // then call the bwdPropFunc() with the differential parts of the parameters &
    // write back any output derivatives.
    //
    auto bwdDiffFunc = builder->createFunc();
    bwdDiffFunc->setFullType(this->bwdDiffFuncType);

    // TODO: do all the decorator and naming stuff here.

    builder->setInsertInto(bwdDiffFunc);
    builder->emitBlock();
    List<IRInst*> bwdDiffFuncParams;
    // Emit parameters for the backward derivative function.
    for (auto paramType : this->bwdDiffFuncType->getParamTypes())
    {
        // TODO: figure out how to put the right names for the parameters.
        auto param = builder->emitParam(paramType);
        bwdDiffFuncParams.add(param);
    }

    auto applyBwdFuncType = cast<IRFuncType>(this->applyBwdFunc->getDataType());
    auto bwdPropFuncType = cast<IRFuncType>(this->bwdPropFunc->getDataType());
    List<IRInst*> applyBwdFuncArgs;
    List<IRInst*> bwdPropFuncParams;

    // TODO: This logic is annoyingly confusing.. rewrite as a switch-case.
    UIndex bwdDiffParamIdx = 0;
    for (UIndex i = 0; i < applyBwdFuncType->getParamCount(); i++)
    {
        // auto applyParamType = this->applyBwdFunc->getParamType(i);
        /*auto bwdPropParamType =
            this->bwdPropFunc->getParamType(i + 1); // +1 to skip the context param*/
        auto applyParamType = applyBwdFuncType->getParamType(i);
        auto bwdPropParamType =
            bwdPropFuncType->getParamType(i + 1); // +1 to skip the context param

        if (as<IRVoidType>(bwdPropParamType))
        {
            bwdPropFuncParams.add(builder->getVoidValue());
        }

        if (as<IROutParamType>(applyParamType))
        {
            // There won't be any parameter in the legacy bwd_diff function for this parameter.
            applyBwdFuncArgs.add(
                builder->emitVar(as<IRPtrTypeBase>(applyParamType)->getValueType()));

            if (!as<IRVoidType>(bwdPropParamType))
            {
                bwdPropFuncParams.add(
                    bwdDiffFuncParams[bwdDiffParamIdx]); // Use the original parameter as-is.
                bwdDiffParamIdx++;
            }
            continue;
        }
        else if (as<IRBorrowInOutParamType>(applyParamType) && as<IRVoidType>(bwdPropParamType))
        {
            auto var = builder->emitVar(as<IRPtrTypeBase>(applyParamType)->getValueType());
            applyBwdFuncArgs.add(var);
            builder->emitStore(var, bwdDiffFuncParams[bwdDiffParamIdx]);
            bwdDiffParamIdx++;
            continue;
        }
        else if (!as<IRVoidType>(bwdPropParamType))
        {
            // TODO: STOPPED HERE: Handle inout no-diff parameters.

            // inout diff-pair or in diff-ptr-pair
            if (auto bwdDiffParamPtrType =
                    as<IRPtrTypeBase>(this->bwdDiffFuncType->getParamType(bwdDiffParamIdx)))
            {
                // as<IRDifferentialPairType>(bwdParamPtrType);
                if (auto applyParamPtrType = as<IRPtrTypeBase>(applyParamType))
                {
                    /*applyBwdFuncArgs.add(builder->emitIntrinsicInst(
                        builder->getPtrType(applyParamPtrType->getValueType()),
                        kIROp_DifferentialPairGetPrimal,
                        1,
                        &bwdDiffFuncParams[bwdDiffParamIdx]));*/

                    // The legacy bwd_diff function should not modify the primal values,
                    // so we'll create a local var and load the primal into it.
                    //
                    auto var = builder->emitVar(as<IRPtrTypeBase>(applyParamType)->getValueType());
                    applyBwdFuncArgs.add(var);
                    builder->emitStore(
                        var,
                        builder->emitLoad(builder->emitIntrinsicInst(
                            builder->getPtrType(applyParamPtrType->getValueType()),
                            kIROp_DifferentialPairGetPrimal,
                            1,
                            &bwdDiffFuncParams[bwdDiffParamIdx])));
                }
                else
                {
                    applyBwdFuncArgs.add(builder->emitLoad(builder->emitIntrinsicInst(
                        builder->getPtrType(applyParamType),
                        kIROp_DifferentialPairGetPrimal,
                        1,
                        &bwdDiffFuncParams[bwdDiffParamIdx])));
                }
                // applyBwdFuncArgs.add(builder->emitLoad(builder->emitDifferentialPairGetPrimal(
                //     bwdDiffFuncParams[bwdDiffParamIdx++]))); // get the primal part

                if (auto bwdPropParamPtrType = as<IRPtrTypeBase>(bwdPropParamType))
                {
                    bwdPropFuncParams.add(builder->emitIntrinsicInst(
                        builder->getPtrType(bwdPropParamPtrType->getValueType()),
                        kIROp_DifferentialPairGetDifferential,
                        1,
                        &bwdDiffFuncParams[bwdDiffParamIdx]));
                }
                else
                {
                    bwdPropFuncParams.add(builder->emitLoad(builder->emitIntrinsicInst(
                        bwdPropParamType,
                        kIROp_DifferentialPairGetDifferential,
                        1,
                        &bwdDiffFuncParams[bwdDiffParamIdx])));
                }
                bwdDiffParamIdx++;
            }
            else
            {
                SLANG_UNEXPECTED("Unexpected parameter type in backward diff transcriber");
            }
        }
        else
        {
            applyBwdFuncArgs.add(bwdDiffFuncParams[bwdDiffParamIdx]);
            bwdDiffParamIdx++;
        }
    }

    // Do we have a left over parameter? This should be the
    // d_Out parameter.
    //
    if (bwdDiffFuncParams.getCount() > bwdDiffParamIdx)
    {
        bwdPropFuncParams.add(bwdDiffFuncParams[bwdDiffParamIdx]);
    }

    auto contextVal = builder->emitCallInst(
        applyBwdFuncType->getResultType(),
        this->applyBwdFunc,
        applyBwdFuncArgs.getCount(),
        applyBwdFuncArgs.getBuffer());
    bwdPropFuncParams.insert(0, contextVal);

    builder->emitCallInst(
        bwdPropFuncType->getResultType(),
        this->bwdPropFunc,
        bwdPropFuncParams.getCount(),
        bwdPropFuncParams.getBuffer());

    builder->emitReturn();

    return {bwdDiffFunc};
}

LegacyToNewBackwardDiffTranslationFuncContext::Result LegacyToNewBackwardDiffTranslationFuncContext::
    translate(IRBuilder* builder)
{
    // We just need to call the applyBwdFunc() with all the primal parts of the parameters
    // then call the bwdPropFunc() with the differential parts of the parameters &
    // write back any output derivatives.
    //

    // Create the context type first (since the rest depend on it).
    auto contextType = builder->createStructType();

    auto applyFunc = builder->createFunc();
    auto bwdPropFunc = builder->createFunc();
    auto getValFunc = builder->createFunc();

    auto legacyBwdDiffFuncType = as<IRFuncType>(this->legacyBwdDiffFunc->getDataType());

    auto outerParent = as<IRGeneric>(findOuterGeneric(primalFunc));

    auto primalFuncType = cast<IRFuncType>(this->primalFunc->getDataType());

    List<IRInst*> applyForBwdFuncTypeParams;
    applyForBwdFuncTypeParams.add(primalFunc->getDataType());
    applyForBwdFuncTypeParams.add(contextType);
    auto applyForBwdFuncType = cast<IRFuncType>(diffTypeContext.resolveType(
        builder,
        builder->emitIntrinsicInst(
            builder->getTypeKind(),
            kIROp_ApplyForBwdFuncType,
            applyForBwdFuncTypeParams.getCount(),
            applyForBwdFuncTypeParams.getBuffer())));

    List<IRInst*> bwdPropFuncTypeParams;
    bwdPropFuncTypeParams.add(primalFunc->getDataType());
    bwdPropFuncTypeParams.add(contextType);
    auto bwdPropFuncType = cast<IRFuncType>(diffTypeContext.resolveType(
        builder,
        builder->emitIntrinsicInst(
            builder->getTypeKind(),
            kIROp_BwdCallableFuncType,
            bwdPropFuncTypeParams.getCount(),
            bwdPropFuncTypeParams.getBuffer())));

    applyFunc->setFullType(applyForBwdFuncType);
    bwdPropFunc->setFullType(bwdPropFuncType);

    // TODO: do all the decorator and naming stuff here.

    IRBuilder applyFuncBuilder(builder->getModule());
    applyFuncBuilder.setInsertInto(applyFunc);
    applyFuncBuilder.emitBlock();
    auto contextVar = applyFuncBuilder.emitVar(contextType);

    IRBuilder contextTypeBuilder(builder->getModule());
    contextTypeBuilder.setInsertInto(builder->getModule());

    IRBuilder bwdPropFuncBuilder(builder->getModule());
    bwdPropFuncBuilder.setInsertInto(bwdPropFunc);
    bwdPropFuncBuilder.emitBlock();
    auto contextInParam =
        bwdPropFuncBuilder.emitParam(contextType); // Context parameter for the bwd prop func.
    bwdPropFuncBuilder.addNameHintDecoration(contextInParam, UnownedStringSlice("ctx"));

    IRBuilder bwdPropPostCallBuilder(builder->getModule());
    bwdPropPostCallBuilder.setInsertAfter(contextInParam);
    auto placeholderCall = bwdPropPostCallBuilder.emitCallInst(
        legacyBwdDiffFuncType->getResultType(),
        legacyBwdDiffFunc,
        0,
        nullptr);

    bwdPropFuncBuilder.setInsertBefore(placeholderCall);

    // Pull up a list of primal params, so we can use them for naming &
    // location tagging.
    //
    ShortList<IRParam*, 8> primalFuncParams;
    auto funcForNames = as<IRFunc>(getResolvedInstForDecorations(primalFunc));
    for (auto param : funcForNames->getParams())
    {
        primalFuncParams.add(param);
    }

    // Jointly emit parameters for the apply and bwd prop functions, while
    // also building the context type.
    //
    List<IRInst*> bwdDiffFuncArgs;
    for (UIndex idx = 0; idx < applyForBwdFuncType->getParamCount(); idx++)
    {
        auto applyForBwdParam = applyFuncBuilder.emitParam(applyForBwdFuncType->getParamType(idx));
        generateName(builder, primalFuncParams[idx], applyForBwdParam, "");
        applyForBwdParam->sourceLoc = primalFuncParams[idx]->sourceLoc;

        auto bwdPropParam = bwdPropFuncBuilder.emitParam(
            bwdPropFuncType->getParamType(idx + 1)); // +1 to skip the context param
        generateName(builder, primalFuncParams[idx], bwdPropParam, "d_");
        bwdPropParam->sourceLoc = primalFuncParams[idx]->sourceLoc;

        if (!as<IROutParamType>(applyForBwdParam->getDataType()))
        {
            auto key = contextTypeBuilder.createStructKey();
            auto structFieldType = applyForBwdParam->getDataType();

            if (auto inoutParamType = as<IRBorrowInOutParamType>(applyForBwdParam->getDataType()))
            {
                structFieldType = inoutParamType->getValueType();
                contextTypeBuilder.createStructField(contextType, key, structFieldType);
            }
            else
            {
                // Has to be "in" type.
                contextTypeBuilder.createStructField(contextType, key, structFieldType);
            }

            applyFuncBuilder.emitStore(
                applyFuncBuilder
                    .emitFieldAddress(builder->getPtrType(structFieldType), contextVar, key),
                applyForBwdParam);

            if (as<IRVoidType>(bwdPropParam->getDataType()))
            {
                // Add just the primal part (there's no differential part since its void).
                bwdDiffFuncArgs.add(
                    bwdPropFuncBuilder.emitFieldExtract(structFieldType, contextInParam, key));
            }
            else
            {
                // If this is not a void type, we need to construct a differential pair
                // var.
                //
                auto inOutPairType = cast<IRBorrowInOutParamType>(
                    legacyBwdDiffFuncType->getParamType(bwdDiffFuncArgs.getCount()));
                IRInst* pairVar = bwdPropFuncBuilder.emitVar(inOutPairType->getValueType());

                // Load the primal value from the context param and store it in here.
                bwdPropFuncBuilder.emitStore(
                    bwdPropFuncBuilder.emitIntrinsicInst(
                        builder->getPtrType(structFieldType),
                        kIROp_DifferentialPairGetPrimal,
                        1,
                        &pairVar),
                    bwdPropFuncBuilder.emitFieldExtract(structFieldType, contextInParam, key));

                auto diffPtr = bwdPropFuncBuilder.emitIntrinsicInst(
                    bwdPropFuncBuilder.getPtrType(
                        as<IROutParamTypeBase>(bwdPropParam->getDataType())->getValueType()),
                    kIROp_DifferentialPairGetDifferential,
                    1,
                    &pairVar);

                if (as<IRBorrowInOutParamType>(bwdPropParam->getDataType()))
                {
                    bwdPropFuncBuilder.emitStore(
                        diffPtr,
                        bwdPropFuncBuilder.emitLoad(bwdPropParam));
                }

                // After the bwdDiff call, load the differential value and put it in bwdPropParam.
                bwdPropPostCallBuilder.emitStore(
                    bwdPropParam,
                    bwdPropPostCallBuilder.emitLoad(diffPtr));

                bwdDiffFuncArgs.add(pairVar);
            }
        }
        else if (!as<IRVoidType>(bwdPropParam->getDataType()))
        {
            // Primal => Out param
            // Diff => In diff param.
            //
            bwdDiffFuncArgs.add(bwdPropParam);
        }
        else
        {
            // Primal => Out param
            // Diff => Void.

            // Nothing to do.
        }
    }

    //
    // Build the getVal() function.
    //

    auto getValFuncType = builder->getFuncType(
        {contextType},
        as<IRFuncType>(primalFunc->getDataType())->getResultType());

    // Emit a call to the primal-func & store the result in a new key,
    // then load that key in the getValFunc and return it.
    //
    IRStructKey* resultKeyInst = contextTypeBuilder.createStructKey();
    auto resultFieldType = as<IRFuncType>(primalFunc->getDataType())->getResultType();
    auto returnValueContextField =
        contextTypeBuilder.createStructField(contextType, resultKeyInst, resultFieldType);
    contextTypeBuilder.addReturnValueContextFieldDecoration(returnValueContextField);

    getValFunc->setFullType(getValFuncType);
    IRBuilder getValFuncBuilder(builder->getModule());
    getValFuncBuilder.setInsertInto(getValFunc);
    getValFuncBuilder.emitBlock();
    auto getValContextParam = getValFuncBuilder.emitParam(contextType);
    getValFuncBuilder.addNameHintDecoration(getValContextParam, UnownedStringSlice("ctx"));

    if (!as<IRVoidType>(resultFieldType))
    {
        // Load the result value from the context and return it
        auto resultVal =
            getValFuncBuilder.emitFieldExtract(resultFieldType, getValContextParam, resultKeyInst);
        getValFuncBuilder.emitReturn(resultVal);
    }
    else
    {
        getValFuncBuilder.emitReturn();
    }

    // Now we need to emit the call to the primal function in the apply function
    List<IRInst*> primalFuncArgs;

    List<IRParam*> applyFuncParams;
    for (auto param : applyFunc->getParams())
    {
        applyFuncParams.add(param);
    }

    for (UIndex ii = 0; ii < applyForBwdFuncType->getParamCount(); ii++)
    {
        auto param = applyFuncParams[ii];
        auto primalFuncParamType = primalFuncType->getParamType(ii);

        if (!diffTypeContext.tryGetAssociationOfKind(
                primalFuncParamType,
                ValAssociationKind::DifferentialPtrType))
        {
            // Simple case: just pass the param as-is.
            primalFuncArgs.add(param);
        }
        else
        {
            // Our param is a ptr-like type that has a differential component, so the param
            // represents a pair.
            auto [paramPassingMode, paramBaseType] =
                splitParameterDirectionAndType(param->getDataType());

            // Handle the other modes later.
            SLANG_ASSERT(paramPassingMode == ParameterDirectionInfo::Kind::In);

            // We need to extract the primal part of the pair and pass that.
            primalFuncArgs.add(
                applyFuncBuilder.emitDifferentialPtrPairGetPrimal(primalFuncParamType, param));
        }
    }

    // Call the primal function and store the result in the context
    auto primalResult = applyFuncBuilder.emitCallInst(
        as<IRFuncType>(primalFunc->getDataType())->getResultType(),
        primalFunc,
        primalFuncArgs.getCount(),
        primalFuncArgs.getBuffer());

    if (!as<IRVoidType>(resultFieldType))
    {
        applyFuncBuilder.emitStore(
            applyFuncBuilder.emitFieldAddress(
                applyFuncBuilder.getPtrType(resultFieldType),
                contextVar,
                resultKeyInst),
            primalResult);
    }

    //
    // Finish up applyFunc & bwdPropFunc.
    //

    applyFuncBuilder.emitReturn(applyFuncBuilder.emitLoad(contextVar));

    if (legacyBwdDiffFuncType->getParamCount() > bwdDiffFuncArgs.getCount())
    {
        // We have a d_out parameter.
        auto dOutParamType = legacyBwdDiffFuncType->getParamType(bwdDiffFuncArgs.getCount());
        auto dOutParam = bwdPropFuncBuilder.emitParam(dOutParamType);
        builder->addNameHintDecoration(dOutParam, UnownedStringSlice("d_out"));
        bwdDiffFuncArgs.add(dOutParam);
    }

    // Replace the placeholder call with the actual bwd diff func call.
    bwdPropFuncBuilder.setInsertBefore(placeholderCall);
    bwdPropFuncBuilder.emitCallInst(
        legacyBwdDiffFuncType->getResultType(),
        this->legacyBwdDiffFunc,
        bwdDiffFuncArgs.getCount(),
        bwdDiffFuncArgs.getBuffer());

    placeholderCall->removeAndDeallocate();

    bwdPropPostCallBuilder.emitReturn();

    generateName(builder, primalFunc, applyFunc, "s_apply_");
    generateName(builder, primalFunc, bwdPropFunc, "s_bwdProp_");
    generateName(builder, primalFunc, getValFunc, "s_getVal_");
    generateName(builder, primalFunc, contextType, "s_bwdCallableCtx_");

    // Hoist contextType first.
    auto contextTypeGlobalVal = maybeHoistAndSpecialize(*builder, contextType);

    auto applyFuncGlobalVal = maybeHoistAndSpecialize(*builder, applyFunc);
    auto bwdPropFuncGlobalVal = maybeHoistAndSpecialize(*builder, bwdPropFunc);
    auto getValFuncGlobalVal = maybeHoistAndSpecialize(*builder, getValFunc);
    return {applyFuncGlobalVal, contextTypeGlobalVal, getValFuncGlobalVal, bwdPropFuncGlobalVal};
}


IRInst* maybeTranslateLegacyToNewBackwardDerivative(
    AutoDiffSharedContext* sharedContext,
    DiagnosticSink* sink,
    IRBackwardFromLegacyBwdDiffFunc* translateInst)
{
    IRInst* primalFunc = translateInst->getOperand(0);
    IRInst* bwdDiffFunc = translateInst->getOperand(1);

    LegacyToNewBackwardDiffTranslationFuncContext context(
        primalFunc,
        bwdDiffFunc,
        sharedContext,
        sink);

    IRBuilder builder(sharedContext->moduleInst);
    builder.setInsertBefore(translateInst);

    LegacyToNewBackwardDiffTranslationFuncContext::Result translationResult =
        context.translate(&builder);
    return builder.emitMakeTuple(
        {translationResult.applyBwdFunc,
         translationResult.bwdPropFunc,
         translationResult.getValFunc,
         translationResult.contextType});
}

IRInst* maybeTranslateLegacyBackwardDerivative(
    AutoDiffSharedContext* sharedContext,
    DiagnosticSink* sink,
    IRLegacyBackwardDifferentiate* translateInst)
{
    IRInst* applyBwdFunc = translateInst->getOperand(0);
    IRInst* contextType = translateInst->getOperand(1);
    IRInst* bwdPropFunc = translateInst->getOperand(2);
    IRFuncType* bwdDiffFuncType = cast<IRFuncType>(translateInst->getDataType());

    LegacyBackwardDiffTranslationFuncContext
        context(applyBwdFunc, contextType, bwdPropFunc, bwdDiffFuncType, sharedContext, sink);

    IRBuilder builder(sharedContext->moduleInst);

    // This will nest the func at the right place (inside any generic contexts).
    builder.setInsertAfter(translateInst);

    LegacyBackwardDiffTranslationFuncContext::Result translationResult =
        context.translate(&builder);
    return translationResult.bwdDiffFunc;
}

IRInst* maybeTranslateBackwardDerivative(
    AutoDiffSharedContext* sharedContext,
    DiagnosticSink* sink,
    IRBackwardDifferentiate* translateInst)
{
    // TODO: This is a temporary redirect into the old solution.. once we
    // know things work, we can just move the logic into this class.

    // Do the reverse-mode translation & return the 4-tuple result.
    BackwardDiffPropagateTranscriber transcriber(sharedContext, sink);
    IRBuilder builder(sharedContext->moduleInst);

    auto baseFunc = translateInst->getOperand(0);
    if (as<IRGeneric>(translateInst->getOperand(0)))
    {
        baseFunc = getGenericReturnVal(translateInst->getOperand(0));
    }

    if (!as<IRFunc>(baseFunc))
        return translateInst;

    auto targetFunc = cast<IRFunc>(baseFunc);

    IRInst* bwdPrimalFunc;
    IRInst* bwdPropagateFunc;
    IRInst* bwdContextGetValFunc;
    IRInst* bwdContextType;
    transcriber._transcribeFuncImpl(
        &builder,
        targetFunc,
        bwdPrimalFunc,
        bwdPropagateFunc,
        bwdContextGetValFunc,
        bwdContextType,
        false);

    builder.setInsertAfter(translateInst);
    return builder.emitMakeTuple(
        {bwdPrimalFunc, bwdPropagateFunc, bwdContextGetValFunc, (IRType*)bwdContextType});
}


IRInst* maybeTranslateTrivialBackwardDerivative(
    AutoDiffSharedContext* sharedContext,
    DiagnosticSink* sink,
    IRTrivialBackwardDifferentiate* translateInst)
{
    // TODO: This is a temporary redirect into the old solution.. once we
    // know things work, we can just move the logic into this class.

    // Do the reverse-mode translation & return the 4-tuple result.
    BackwardDiffPropagateTranscriber transcriber(sharedContext, sink);
    IRBuilder builder(sharedContext->moduleInst);

    auto baseFunc = translateInst->getOperand(0);
    if (as<IRGeneric>(translateInst->getOperand(0)))
    {
        baseFunc = getGenericReturnVal(translateInst->getOperand(0));
    }

    if (!as<IRFunc>(baseFunc))
        return translateInst;

    auto targetFunc = cast<IRFunc>(baseFunc);

    IRInst* bwdPrimalFunc;
    IRInst* bwdPropagateFunc;
    IRInst* bwdContextGetValFunc;
    IRInst* bwdContextType;
    transcriber._transcribeFuncImpl(
        &builder,
        targetFunc,
        bwdPrimalFunc,
        bwdPropagateFunc,
        bwdContextGetValFunc,
        bwdContextType,
        true);

    builder.setInsertAfter(translateInst);
    return builder.emitMakeTuple(
        {bwdPrimalFunc, bwdPropagateFunc, bwdContextGetValFunc, (IRType*)bwdContextType});
}

} // namespace Slang
