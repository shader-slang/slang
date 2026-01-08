// slang-ir-autodiff-fwd.h
#pragma once

#include "slang-ir-autodiff-transcriber-base.h"
#include "slang-ir-specialize.h"

namespace Slang
{

struct ForwardDiffTranscriber : AutoDiffTranscriberBase
{
    // Pending values to write back to inout params at the end of the current function.
    OrderedDictionary<IRInst*, InstPair> mapInOutParamToWriteBackValue;

    // Signals the transcriber to use an IRTrivialForwardDifferentiate(func)
    // instruction for functions that are non-forward-differentiable, but
    // have a reverse-mode derivative defined.
    //
    // Normally, we just treat the call as non-differentiable, but this will
    // cause the reverse-mode differentiation step to skip the function as well
    //
    bool useTrivialFwdsForBwdDifferentiableFuncs = true;

    void enableReverseModeCompatibility()
    {
        useTrivialFwdsForBwdDifferentiableFuncs = true;

        // TODO: Any other flags that we need to generate a reverse-mode compatible
        // forward-mode derivative.
    }

    ForwardDiffTranscriber(AutoDiffSharedContext* shared, DiagnosticSink* inSink)
        : AutoDiffTranscriberBase(shared, inSink)
    {
    }

    IRFuncType* resolveFuncType(IRBuilder* builder, IRInst* funcType);

    // Returns "d<var-name>" to use as a name hint for variables and parameters.
    // If no primal name is available, returns a blank string.
    //
    String getJVPVarName(IRInst* origVar);

    // Returns "dp<var-name>" to use as a name hint for parameters.
    // If no primal name is available, returns a blank string.
    //
    String makeDiffPairName(IRInst* origVar);

    InstPair transcribeUndefined(IRBuilder* builder, IRInst* origInst);

    InstPair transcribeVar(IRBuilder* builder, IRVar* origVar);

    InstPair transcribeBinaryArith(IRBuilder* builder, IRInst* origArith);

    InstPair transcribeBinaryLogic(IRBuilder* builder, IRInst* origLogic);

    InstPair transcribeSelect(IRBuilder* builder, IRInst* origSelect);

    InstPair transcribeLoad(IRBuilder* builder, IRLoad* origLoad);

    InstPair transcribeStore(IRBuilder* builder, IRStore* origStore);

    // Since int/float literals are sometimes nested inside an IRConstructor
    // instruction, we check to make sure that the nested instr is a constant
    // and then return nullptr. Literals do not need to be differentiated.
    //
    InstPair transcribeConstruct(IRBuilder* builder, IRInst* origConstruct);
    InstPair transcribeMakeStruct(IRBuilder* builder, IRInst* origMakeStruct);

    InstPair transcribeMakeTuple(IRBuilder* builder, IRInst* origMakeTuple);

    // Differentiating a call instruction here is primarily about generating
    // an appropriate call list based on whichever parameters have differentials
    // in the current transcription context.
    //
    InstPair transcribeCall(IRBuilder* builder, IRCall* origCall);

    InstPair transcribeSwizzle(IRBuilder* builder, IRSwizzle* origSwizzle);

    InstPair transcribeByPassthrough(IRBuilder* builder, IRInst* origInst);

    InstPair transcribeControlFlow(IRBuilder* builder, IRInst* origInst);

    InstPair transcribeConst(IRBuilder* builder, IRInst* origInst);

    InstPair transcribeSpecialize(IRBuilder* builder, IRSpecialize* origSpecialize);

    InstPair transcribeFieldExtract(IRBuilder* builder, IRInst* originalInst);

    InstPair transcribeGetElement(IRBuilder* builder, IRInst* origGetElementPtr);

    InstPair transcribeGetTupleElement(IRBuilder* builder, IRInst* origInst);

    InstPair transcribeGetOptionalValue(IRBuilder* builder, IRInst* originalInst);

    InstPair transcribeUpdateElement(IRBuilder* builder, IRInst* originalInst);

    InstPair transcribeIfElse(IRBuilder* builder, IRIfElse* origIfElse);

    InstPair transcribeSwitch(IRBuilder* builder, IRSwitch* origSwitch);

    InstPair transcribeMakeDifferentialPair(IRBuilder* builder, IRInst* origInst);

    InstPair transcribeMakeExistential(IRBuilder* builder, IRMakeExistential* origMakeExistential);

    InstPair transcribeDifferentialPairGetElement(IRBuilder* builder, IRInst* origInst);

    InstPair transcribeSingleOperandInst(IRBuilder* builder, IRInst* origInst);

    InstPair transcribeWrapExistential(IRBuilder* builder, IRInst* origInst);

    InstPair transcribeDefaultConstruct(IRBuilder* builder, IRInst* origInst);

    InstPair transcribeReinterpret(IRBuilder* builder, IRInst* origInst);

    InstPair transcribeDifferentiableTypeAnnotation(IRBuilder* builder, IRInst* origInst);

    InstPair transcribeAssociatedInstAnnotation(IRBuilder* builder, IRInst* origInst);

    virtual IRFuncType* differentiateFunctionType(
        IRBuilder* builder,
        IRInst* func,
        IRFuncType* funcType) override;

    void generateTrivialFwdDiffFunc(IRFunc* primalFunc, IRFunc* diffFunc);

    // Transcribe a function definition.
    InstPair transcribeFunc(IRBuilder* inBuilder, IRFunc* primalFunc, IRFunc* diffFunc);

    // Transcribe a function without marking the result as a decoration.
    IRFunc* transcribeFuncHeaderImpl(IRBuilder* inBuilder, IRFunc* origFunc);

    void checkAutodiffInstDecorations(IRFunc* fwdFunc);

    SlangResult prepareFuncForForwardDiff(IRFunc* func);

    void _transcribeFuncImpl(IRBuilder* inBuilder, IRInst* origInst, IRInst*& fwdDiffFunc);

    // Create an empty func to represent the transcribed func of `origFunc`.
    virtual InstPair transcribeFuncHeader(IRBuilder* inBuilder, IRFunc* origFunc) override;

    virtual InstPair transcribeInstImpl(IRBuilder* builder, IRInst* origInst) override;

    virtual InstPair transcribeFuncParam(IRBuilder* builder, IRParam* origParam, IRInst* primalType)
        override;

    virtual IROp getInterfaceRequirementDerivativeDecorationOp() override
    {
        return kIROp_ForwardDerivativeDecoration;
    }
};

struct ForwardDiffTranslationFuncContext
{
    struct Result
    {
        IRInst* fwdDiffFunc = nullptr;
    };

    // Shared context holding on to interface definitions, etc..
    AutoDiffSharedContext* sharedContext;

    // Differentiable type conformance context.
    DifferentiableTypeConformanceContext diffTypeContext;

    // The function to transcribe.
    IRFunc* targetFunc;

    // Whether the function is a trivial case (zero derivatives).
    bool isTrivial = false;

    // The diagnostic sink to report errors.
    DiagnosticSink* sink;

    ForwardDiffTranslationFuncContext(
        IRFunc* targetFunc,
        bool isTrivial,
        AutoDiffSharedContext* shared,
        DiagnosticSink* sink)
        : sharedContext(shared)
        , diffTypeContext(shared)
        , targetFunc(targetFunc)
        , isTrivial(isTrivial)
        , sink(sink)
    {
        diffTypeContext.setFunc(targetFunc);
    }

    Result translate(IRBuilder* builder)
    {
        // TODO: This is a temporary redirect into the old solution.. once we
        // know things work, we can just move the logic into this class.

        // Do the reverse-mode translation & return the 4-tuple result.
        ForwardDiffTranscriber transcriber(sharedContext, sink);
        if (!isTrivial)
        {
            IRInst* fwdDiffFunc;
            transcriber._transcribeFuncImpl(builder, targetFunc, fwdDiffFunc);

            return {fwdDiffFunc};
        }
        else
        {
            IRInst* fwdDiffFunc = builder->createFunc();
            IRInst* typeOperand = targetFunc->getFullType();
            fwdDiffFunc->setFullType(diffTypeContext.resolveType(
                builder,
                builder->emitIntrinsicInst(
                    builder->getTypeKind(),
                    kIROp_ForwardDiffFuncType,
                    1,
                    &typeOperand)));
            transcriber.generateTrivialFwdDiffFunc(targetFunc, cast<IRFunc>(fwdDiffFunc));
            return {fwdDiffFunc};
        }
    }
};

// AD 2.0 version.
struct ForwardDiffTranslator
{
    // Keep track of global insts that have already been translated.
    Dictionary<IRInst*, ForwardDiffTranslationFuncContext::Result> translationCache;

    IRInst* getBaseForTranslateInst(IRInst* inst)
    {
        switch (inst->getOp())
        {
        case kIROp_ForwardDifferentiate:
            return inst->getOperand(0);
        default:
            return nullptr;
        }
    }

    // TODO: MERGE
    struct TranslationBaseInfo
    {
        List<IRInst*> operands;

        enum Flavor
        {
            Unknown,
            Simple,    // Direct reference to func
            Specialize // Indirect reference that requires specialization.
        } flavor = Flavor::Unknown;

        List<IRInst*> specializeOperands;
    };
    // TODO: MERGE
    IRInst* materialize(IRBuilder* builder, TranslationBaseInfo& info, IRInst* resultInst)
    {
        if (info.flavor == TranslationBaseInfo::Flavor::Simple)
        {
            // Simple case, just return the function.
            return resultInst;
        }
        else if (info.flavor == TranslationBaseInfo::Flavor::Specialize)
        {
            auto outerGeneric = resultInst;
            SLANG_ASSERT(as<IRGeneric>(resultInst));
            auto innerVal = getGenericReturnVal(resultInst);
            if (as<IRFunc>(innerVal) || innerVal->getDataType()->getOp() == kIROp_FuncType)
            {
                return specializeGeneric(
                    as<IRSpecialize>(builder->emitSpecializeInst(
                        (IRType*)specializeGeneric(
                            as<IRSpecialize>(builder->emitSpecializeInst(
                                builder->getTypeKind(),
                                resultInst->getDataType(),
                                info.specializeOperands))),
                        resultInst,
                        info.specializeOperands)));
            }
            else if (as<IRStructType>(innerVal))
            {
                return (IRType*)specializeGeneric(
                    as<IRSpecialize>(builder->emitSpecializeInst(
                        builder->getTypeKind(),
                        resultInst,
                        info.specializeOperands)));
            }
            else
            {
                SLANG_UNEXPECTED("Unexpected result inst type for specialization.");
            }
        }
        else
        {
            SLANG_UNEXPECTED("Unknown translation request flavor.");
        }
    }
    // TODO: MERGE
    bool addOperand(TranslationBaseInfo& baseInfo, IRInst* operand)
    {
        if (operand)
        {
            if (auto funcInst = as<IRFunc>(operand))
            {
                baseInfo.operands.add(funcInst);
                if (baseInfo.operands.getCount() == 1)
                {
                    baseInfo.flavor = TranslationBaseInfo::Flavor::Simple;
                }
                else if (baseInfo.flavor != TranslationBaseInfo::Flavor::Simple)
                    return false; // Can't unify with existing operand.

                return true;
            }
            else if (auto specializeInst = as<IRSpecialize>(operand))
            {
                // If the base is a specialization, we need to collect the operands.
                IRGeneric* genericInst = cast<IRGeneric>(specializeInst->getBase());
                baseInfo.operands.add(genericInst);

                if (baseInfo.operands.getCount() == 1)
                {
                    for (UInt i = 0; i < specializeInst->getArgCount(); i++)
                        baseInfo.specializeOperands.add(specializeInst->getArg(i));
                    baseInfo.flavor = TranslationBaseInfo::Flavor::Specialize;
                }
                else if (baseInfo.flavor != TranslationBaseInfo::Flavor::Specialize)
                    return false; // Can't unify with existing operand.
                else
                {
                    // check that the operands match
                    if (baseInfo.specializeOperands.getCount() != specializeInst->getArgCount())
                        return false; // Can't unify with existing operands.
                    for (UInt i = 0; i < baseInfo.specializeOperands.getCount(); i++)
                    {
                        if (baseInfo.specializeOperands[i] != specializeInst->getArg(i))
                            return false; // Can't unify with existing operands.
                    }
                }

                return true;
            }
            else
            {
                return false;
            }
        }
    }

    TranslationBaseInfo getTranslationBaseInfo(IRInst* inst)
    {
        IRInst* primalFunc = inst->getOperand(0);

        TranslationBaseInfo info;
        bool valid = true;
        valid &= addOperand(info, primalFunc);

        if (!valid)
            return TranslationBaseInfo();

        return info;
    }

    IRInst* processTranslationRequest(
        IRInst* translateInst,
        AutoDiffSharedContext* sharedContext,
        DiagnosticSink* sink)
    {
        auto baseInfo = getTranslationBaseInfo(translateInst);

        /*auto globalValToTranslate = as<IRFunc>(getResolvedInstForDecorations(baseInst));
        if (!globalValToTranslate)
        {
            // TODO: diagnose
            SLANG_UNEXPECTED("Expected a global value with code for forward differentiation.");
        }*/

        IRBuilder builder(sharedContext->moduleInst);
        auto translationKey = baseInfo.operands[0];

        if (translationCache.containsKey(translationKey))
        {
            // If we already have a translation for this function, return the requested value.
            return materialize(&builder, baseInfo, translationCache[translationKey].fwdDiffFunc);
        }

        IRFunc* funcToTranslate =
            (as<IRGeneric>(translationKey) ? cast<IRFunc>(getGenericReturnVal(translationKey))
                                           : cast<IRFunc>(translationKey));

        // Intrinsics are not currently explicitly differentiated.
        //
        // TODO: What about intrinsics that are accessed via dynamic dispatch?
        //
        if (funcToTranslate->findDecoration<IRIntrinsicOpDecoration>())
            return nullptr;

        // Create a new context for the translation.
        ForwardDiffTranslationFuncContext context(funcToTranslate, false, sharedContext, sink);
        builder.setInsertAfter(funcToTranslate);

        // Translate, cache and return the result.
        ForwardDiffTranslationFuncContext::Result translationResult = context.translate(&builder);
        translationCache.add(translationKey, translationResult);
        return materialize(&builder, baseInfo, translationResult.fwdDiffFunc);
    }
};

// TODO: De-duplicate
struct TrivialForwardDiffTranslator
{
    // Keep track of global insts that have already been translated.
    Dictionary<IRInst*, ForwardDiffTranslationFuncContext::Result> translationCache;

    // TODO: MERGE
    struct TranslationBaseInfo
    {
        List<IRInst*> operands;

        enum Flavor
        {
            Unknown,
            Simple,    // Direct reference to func
            Specialize // Indirect reference that requires specialization.
        } flavor = Flavor::Unknown;

        List<IRInst*> specializeOperands;
    };
    // TODO: MERGE
    IRInst* materialize(IRBuilder* builder, TranslationBaseInfo& info, IRInst* resultInst)
    {
        if (info.flavor == TranslationBaseInfo::Flavor::Simple)
        {
            // Simple case, just return the function.
            return resultInst;
        }
        else if (info.flavor == TranslationBaseInfo::Flavor::Specialize)
        {
            auto outerGeneric = resultInst;
            SLANG_ASSERT(as<IRGeneric>(resultInst));
            auto innerVal = getGenericReturnVal(resultInst);
            if (as<IRFunc>(innerVal) || innerVal->getDataType()->getOp() == kIROp_FuncType)
            {
                return specializeGeneric(
                    as<IRSpecialize>(builder->emitSpecializeInst(
                        (IRType*)specializeGeneric(
                            as<IRSpecialize>(builder->emitSpecializeInst(
                                builder->getTypeKind(),
                                resultInst->getDataType(),
                                info.specializeOperands))),
                        resultInst,
                        info.specializeOperands)));
            }
            else if (as<IRStructType>(innerVal))
            {
                return (IRType*)specializeGeneric(
                    as<IRSpecialize>(builder->emitSpecializeInst(
                        builder->getTypeKind(),
                        resultInst,
                        info.specializeOperands)));
            }
            else
            {
                SLANG_UNEXPECTED("Unexpected result inst type for specialization.");
            }
        }
        else
        {
            SLANG_UNEXPECTED("Unknown translation request flavor.");
        }
    }
    // TODO: MERGE
    bool addOperand(TranslationBaseInfo& baseInfo, IRInst* operand)
    {
        if (operand)
        {
            if (auto funcInst = as<IRFunc>(operand))
            {
                baseInfo.operands.add(funcInst);
                if (baseInfo.operands.getCount() == 1)
                {
                    baseInfo.flavor = TranslationBaseInfo::Flavor::Simple;
                }
                else if (baseInfo.flavor != TranslationBaseInfo::Flavor::Simple)
                    return false; // Can't unify with existing operand.

                return true;
            }
            else if (auto specializeInst = as<IRSpecialize>(operand))
            {
                // If the base is a specialization, we need to collect the operands.
                IRGeneric* genericInst = cast<IRGeneric>(specializeInst->getBase());
                baseInfo.operands.add(genericInst);

                if (baseInfo.operands.getCount() == 1)
                {
                    for (UInt i = 0; i < specializeInst->getArgCount(); i++)
                        baseInfo.specializeOperands.add(specializeInst->getArg(i));
                    baseInfo.flavor = TranslationBaseInfo::Flavor::Specialize;
                }
                else if (baseInfo.flavor != TranslationBaseInfo::Flavor::Specialize)
                    return false; // Can't unify with existing operand.
                else
                {
                    // check that the operands match
                    if (baseInfo.specializeOperands.getCount() != specializeInst->getArgCount())
                        return false; // Can't unify with existing operands.
                    for (UInt i = 0; i < baseInfo.specializeOperands.getCount(); i++)
                    {
                        if (baseInfo.specializeOperands[i] != specializeInst->getArg(i))
                            return false; // Can't unify with existing operands.
                    }
                }

                return true;
            }
            else
            {
                return false;
            }
        }
    }

    TranslationBaseInfo getTranslationBaseInfo(IRInst* inst)
    {
        IRInst* primalFunc = inst->getOperand(0);

        TranslationBaseInfo info;
        bool valid = true;
        valid &= addOperand(info, primalFunc);

        if (!valid)
            return TranslationBaseInfo();

        return info;
    }

    IRInst* processTranslationRequest(
        IRInst* translateInst,
        AutoDiffSharedContext* sharedContext,
        DiagnosticSink* sink)
    {
        /*auto translationBase = translateInst->getOperand(0);
        if (as<IRFunc>(translationBase))
            return translateInst;
        else if (as<IRGeneric>(translationBase))
            return translateInst;

        // Create a new context for the translation.
        IRBuilder builder(sharedContext->moduleInst);
        ForwardDiffTranslationFuncContext context(translationBase, true, sharedContext, sink);
        builder.setInsertAfter(funcToTranslate);
        auto translationResult = context.translate(&builder);
        return translationResult.fwdDiffFunc;*/

        auto baseInfo = getTranslationBaseInfo(translateInst);

        if (baseInfo.flavor == TranslationBaseInfo::Flavor::Unknown)
            return nullptr;

        IRBuilder builder(sharedContext->moduleInst);
        auto translationKey = baseInfo.operands[0];

        if (translationCache.containsKey(translationKey))
        {
            // If we already have a translation for this function, return the requested value.
            return materialize(&builder, baseInfo, translationCache[translationKey].fwdDiffFunc);
        }

        IRFunc* funcToTranslate =
            (as<IRGeneric>(translationKey) ? cast<IRFunc>(getGenericReturnVal(translationKey))
                                           : cast<IRFunc>(translationKey));

        // Intrinsics are not currently explicitly differentiated.
        //
        // TODO: What about intrinsics that are accessed via dynamic dispatch?
        //
        if (funcToTranslate->findDecoration<IRIntrinsicOpDecoration>())
            return nullptr;

        // Create a new context for the translation.
        ForwardDiffTranslationFuncContext context(funcToTranslate, true, sharedContext, sink);
        builder.setInsertAfter(funcToTranslate);

        // Translate, cache and return the result.
        ForwardDiffTranslationFuncContext::Result translationResult = context.translate(&builder);
        translationCache.add(translationKey, translationResult);
        return materialize(&builder, baseInfo, translationResult.fwdDiffFunc);
    }
};


// This translator supports higher order auto-diff by synthesizing new conformances to
// IForwardDifferentiable and IBackwardDifferentiable wherever necessary.
//
struct ForwardDiffWitnessTranslator
{
    IRInst* processTranslationRequest(
        IRInst* translateInst,
        AutoDiffSharedContext* sharedContext,
        DiagnosticSink* sink)
    {
        return nullptr;
    }
};


IRInst* maybeTranslateForwardDerivative(
    AutoDiffSharedContext* sharedContext,
    DiagnosticSink* sink,
    IRForwardDifferentiate* inst);

IRInst* maybeTranslateTrivialForwardDerivative(
    AutoDiffSharedContext* sharedContext,
    DiagnosticSink* sink,
    IRTrivialForwardDifferentiate* inst);

IRInst* maybeTranslateForwardDerivativeWitness(
    AutoDiffSharedContext* sharedContext,
    DiagnosticSink* sink,
    IRSynthesizedForwardDerivativeWitnessTable* translateInst);

IRInst* maybeTranslateBackwardDerivativeWitness(
    AutoDiffSharedContext* sharedContext,
    DiagnosticSink* sink,
    IRSynthesizedBackwardDerivativeWitnessTable* translateInst);

} // namespace Slang