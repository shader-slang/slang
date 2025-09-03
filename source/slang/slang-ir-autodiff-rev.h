// slang-ir-autodiff-rev.h
#pragma once

#include "slang-compiler.h"
#include "slang-ir-autodiff-fwd.h"
#include "slang-ir-autodiff-propagate.h"
#include "slang-ir-autodiff-transcriber-base.h"
#include "slang-ir-autodiff-transpose.h"
#include "slang-ir-autodiff-unzip.h"
#include "slang-ir-autodiff.h"
#include "slang-ir-insts.h"
#include "slang-ir-specialize.h"
#include "slang-ir.h"

namespace Slang
{

struct IRReverseDerivativePassOptions
{
    // Nothing for now..
};

// The result of function parameter transposition.
// Contains necessary info for future processing in the backward differentation pass.
struct ParameterBlockTransposeInfo
{
    // Parameters that should be in the furture primal function.
    HashSet<IRInst*> primalFuncParams;

    // Parameters that should be in the furture propagate function.
    HashSet<IRInst*> propagateFuncParams;

    // The value with which a primal specific parameter should be replaced in propagate func.
    OrderedDictionary<IRInst*, IRInst*> mapPrimalSpecificParamToReplacementInPropFunc;

    // The insts added that is specific for propagate functions and should be removed
    // from the future primal func.
    List<IRInst*> propagateFuncSpecificPrimalInsts;

    // Write backs to perform at the end of the back-prop function in order to return the
    // computed output derivatives for an inout parameter.
    OrderedDictionary<IRInst*, InstPair> outDiffWritebacks;

    // The dOut parameter representing the result derivative to propagate backwards through.
    IRInst* dOutParam;

    // ---- AD 2.0 fields ----

    // Mapping from insts whose accumulated gradient should be written into
    // a specific param.
    OrderedDictionary<IRInst*, IRInst*> transposedInstMap;

    // At the end of function, write back the value in inst into the param.
    OrderedDictionary<IRParam*, IRInst*> writebacks;
};

struct BackwardDiffTranscriberBase : AutoDiffTranscriberBase
{
    FuncBodyTranscriptionTaskType diffTaskType;

    // References to other passes that for reverse-mode transcription.
    DiffTransposePass* diffTransposePass;
    DiffPropagationPass* diffPropagationPass;
    DiffUnzipPass* diffUnzipPass;

    // Allocate space for the passes.
    DiffTransposePass diffTransposePassStorage;
    DiffPropagationPass diffPropagationPassStorage;
    DiffUnzipPass diffUnzipPassStorage;

    BackwardDiffTranscriberBase(
        FuncBodyTranscriptionTaskType taskType,
        AutoDiffSharedContext* shared,
        DiagnosticSink* inSink)
        : AutoDiffTranscriberBase(shared, inSink)
        , diffTaskType(taskType)
        , diffTransposePassStorage(shared)
        , diffPropagationPassStorage(shared)
        , diffUnzipPassStorage(shared)
        , diffTransposePass(&diffTransposePassStorage)
        , diffPropagationPass(&diffPropagationPassStorage)
        , diffUnzipPass(&diffUnzipPassStorage)
    {
    }

    // Returns "dp<var-name>" to use as a name hint for parameters.
    // If no primal name is available, returns a blank string.
    //
    String makeDiffPairName(IRInst* origVar);

    IRFuncType* differentiateFunctionTypeImpl(
        IRBuilder* builder,
        IRFuncType* funcType,
        IRInst* intermediateType);

    IRType* transcribeParamTypeForPrimalFunc(IRBuilder* builder, IRType* paramType);
    IRType* transcribeParamTypeForPropagateFunc(IRBuilder* builder, IRType* paramType);

    // Puts parameters into their own block.
    void makeParameterBlock(IRBuilder* inBuilder, IRFunc* func);

    // Transcribe a function definition.
    virtual InstPair transcribeFunc(IRBuilder* builder, IRFunc* primalFunc, IRFunc* diffFunc) = 0;

    // Get transcribed function name from original name.
    virtual IRStringLit* getTranscribedFuncName(
        IRBuilder* builder,
        IRGlobalValueWithCode* func) = 0;

    // Splits and transpose the parameter block.
    // After this operation, the parameter block will contain parameters for both the future
    // primal func and the future propagate func.
    // Additional info is returned in `ParameterBlockTransposeInfo` for future processing such
    // as inserting write-back logic or splitting them into different functions.
    ParameterBlockTransposeInfo splitAndTransposeParameterBlock(
        IRBuilder* builder,
        IRFunc* diffFunc,
        SourceLoc primalLoc,
        bool isResultDifferentiable);

    void writeBackDerivativeToInOutParams(ParameterBlockTransposeInfo& info, IRFunc* diffFunc);

    virtual InstPair transcribeFuncParam(IRBuilder* builder, IRParam* origParam, IRInst* primalType)
        override;

    InstPair transcribeSpecialize(IRBuilder* builder, IRSpecialize* origSpecialize);

    SlangResult prepareFuncForBackwardDiff(IRFunc* func);

    IRFunc* generateNewForwardDerivativeForFunc(
        IRBuilder* builder,
        IRFunc* originalFunc,
        IRFunc* diffPropagateFunc);

    void transcribeFuncImpl(IRBuilder* builder, IRFunc* primalFunc, IRFunc* diffPropagateFunc);

    // New interface for AD 2.0
    void _transcribeFuncImpl(
        IRBuilder* builder,
        IRFunc* targetFunc,
        IRInst*& applyFunc,
        IRInst*& propagateFunc,
        IRInst*& contextGetValFunc,
        IRInst*& contextType);

    InstPair transcribeFuncHeaderImpl(IRBuilder* inBuilder, IRFunc* origFunc);

    void addTranscribedFuncDecoration(
        IRBuilder& builder,
        IRFunc* origFunc,
        IRFunc* transcribedFunc);

    virtual InstPair transcribeFuncHeader(IRBuilder* inBuilder, IRFunc* origFunc) override;

    virtual InstPair transcribeInstImpl(IRBuilder* builder, IRInst* origInst) override;

    virtual IRInst* findExistingDiffFunc(IRInst* originalFunc) = 0;
    virtual void addExistingDiffFuncDecor(IRBuilder* builder, IRInst* inst, IRInst* diffFunc) = 0;

    virtual IROp getInterfaceRequirementDerivativeDecorationOp() override
    {
        return kIROp_BackwardDerivativeDecoration;
    }

    // virtual IRInst* processTranslationRequest(IRInst* inst) override;
};

struct BackwardDiffPrimalTranscriber : BackwardDiffTranscriberBase
{
    BackwardDiffPrimalTranscriber(AutoDiffSharedContext* shared, DiagnosticSink* inSink)
        : BackwardDiffTranscriberBase(FuncBodyTranscriptionTaskType::BackwardPrimal, shared, inSink)
    {
    }

    virtual IRFuncType* differentiateFunctionType(
        IRBuilder* builder,
        IRInst* func,
        IRFuncType* funcType) override;
    virtual InstPair transcribeFunc(IRBuilder* builder, IRFunc* primalFunc, IRFunc* diffFunc)
        override;
    virtual IRInst* findExistingDiffFunc(IRInst* originalFunc) override
    {
        if (auto backDecor = originalFunc->findDecoration<IRBackwardDerivativePrimalDecoration>())
        {
            return backDecor->getBackwardDerivativePrimalFunc();
        }
        return nullptr;
    }
    virtual void addExistingDiffFuncDecor(IRBuilder* builder, IRInst* inst, IRInst* diffFunc)
        override
    {
        builder->addBackwardDerivativePrimalDecoration(inst, diffFunc);
    }
    virtual IROp getInterfaceRequirementDerivativeDecorationOp() override
    {
        return kIROp_BackwardDerivativePrimalDecoration;
    }
    virtual IRStringLit* getTranscribedFuncName(IRBuilder* builder, IRGlobalValueWithCode* func)
        override
    {
        if (auto nameHint = func->findDecoration<IRNameHintDecoration>())
        {
            StringBuilder sbuilder;
            sbuilder << "s_primal_ctx_" << nameHint->getName();
            return builder->getStringValue(sbuilder.getUnownedSlice());
        }
        else
        {
            return builder->getStringValue(String("s_primal_ctx_anonymous").getUnownedSlice());
        }
    }

    // AD 2.0 translation-request-based functions.
    /*virtual IRInst* processTranslationRequest(IRInst* inst) override
    {
        auto backwardDiffPrimalInst = cast<IRBackwardDifferentiatePrimal>(inst);
        auto baseFn = as<IRFunc>(backwardDiffPrimalInst->getBaseFn());
        if (!baseFn)
        {
            SLANG_UNEXPECTED("backward-differentiate-primal should have a base function");
        }
    }*/
};


struct BackwardDiffPropagateTranscriber : BackwardDiffTranscriberBase
{
    BackwardDiffPropagateTranscriber(AutoDiffSharedContext* shared, DiagnosticSink* inSink)
        : BackwardDiffTranscriberBase(
              FuncBodyTranscriptionTaskType::BackwardPropagate,
              shared,
              inSink)
    {
    }
    void generateTrivialDiffFuncFromUserDefinedDerivative(
        IRBuilder* builder,
        IRFunc* primalFunc,
        IRFunc* diffPropFunc,
        IRUserDefinedBackwardDerivativeDecoration* udfDecor);

    virtual IRFuncType* differentiateFunctionType(
        IRBuilder* builder,
        IRInst* func,
        IRFuncType* funcType) override;
    virtual InstPair transcribeFunc(IRBuilder* builder, IRFunc* primalFunc, IRFunc* diffFunc)
        override;
    virtual IRInst* findExistingDiffFunc(IRInst* originalFunc) override
    {
        if (auto backDecor =
                originalFunc->findDecoration<IRBackwardDerivativePropagateDecoration>())
        {
            return backDecor->getBackwardDerivativePropagateFunc();
        }
        return nullptr;
    }
    virtual void addExistingDiffFuncDecor(IRBuilder* builder, IRInst* inst, IRInst* diffFunc)
        override
    {
        builder->addBackwardDerivativePropagateDecoration(inst, diffFunc);
    }
    virtual IROp getInterfaceRequirementDerivativeDecorationOp() override
    {
        return kIROp_BackwardDerivativePropagateDecoration;
    }
    virtual IRStringLit* getTranscribedFuncName(IRBuilder* builder, IRGlobalValueWithCode* func)
        override
    {
        if (auto nameHint = func->findDecoration<IRNameHintDecoration>())
        {
            StringBuilder sbuilder;
            sbuilder << "s_bwd_prop_" << nameHint->getName();
            return builder->getStringValue(sbuilder.getUnownedSlice());
        }
        else
        {
            return builder->getStringValue(String("s_bwd_prop_anonymous").getUnownedSlice());
        }
    }
};

// A backward derivative function combines both primal + propagate functions and accepts no
// intermediate value input.
struct BackwardDiffTranscriber : BackwardDiffTranscriberBase
{
    BackwardDiffTranscriber(AutoDiffSharedContext* shared, DiagnosticSink* inSink)
        : BackwardDiffTranscriberBase(FuncBodyTranscriptionTaskType::Backward, shared, inSink)
    {
    }

    virtual IRFuncType* differentiateFunctionType(
        IRBuilder* builder,
        IRInst* func,
        IRFuncType* funcType) override;
    virtual InstPair transcribeFuncHeader(IRBuilder* inBuilder, IRFunc* origFunc) override;
    virtual InstPair transcribeFunc(IRBuilder* builder, IRFunc* primalFunc, IRFunc* diffFunc)
        override
    {
        // Don't need to do anything here, the body is generated in transcribeFuncHeader.

        SLANG_UNUSED(builder);
        addTranscribedFuncDecoration(*builder, primalFunc, diffFunc);
        return InstPair(primalFunc, diffFunc);
    }
    virtual IRInst* findExistingDiffFunc(IRInst* originalFunc) override
    {
        if (auto backDecor = originalFunc->findDecoration<IRBackwardDerivativeDecoration>())
        {
            return backDecor->getBackwardDerivativeFunc();
        }
        if (auto backDecor =
                originalFunc->findDecoration<IRUserDefinedBackwardDerivativeDecoration>())
        {
            return backDecor->getBackwardDerivativeFunc();
        }
        return nullptr;
    }
    virtual void addExistingDiffFuncDecor(IRBuilder* builder, IRInst* inst, IRInst* diffFunc)
        override
    {
        builder->addBackwardDerivativeDecoration(inst, diffFunc);
    }
    virtual IRStringLit* getTranscribedFuncName(IRBuilder* builder, IRGlobalValueWithCode* func)
        override
    {
        if (auto nameHint = func->findDecoration<IRNameHintDecoration>())
        {
            StringBuilder sbuilder;
            sbuilder << "s_bwd_" << nameHint->getName();
            return builder->getStringValue(sbuilder.getUnownedSlice());
        }
        else
        {
            return builder->getStringValue(String("s_bwd_anonymous").getUnownedSlice());
        }
    }
};


// Triggers the actual pass. (AD 2.0)
struct BackwardDiffTranslationFuncContext
{
    struct Result
    {
        IRInst* bwdApplyFunc = nullptr;
        IRInst* bwdPropFunc = nullptr;
        IRInst* bwdValFunc = nullptr;
        IRType* bwdContextType = nullptr;
    };

    // Shared context holding on to interface definitions, etc..
    AutoDiffSharedContext* sharedContext;

    // Differentiable type conformance context.
    DifferentiableTypeConformanceContext diffTypeContext;

    // The function to transcribe.
    IRFunc* targetFunc;

    // The diagnostic sink to report errors.
    DiagnosticSink* sink;

    BackwardDiffTranslationFuncContext(
        IRFunc* targetFunc,
        AutoDiffSharedContext* shared,
        DiagnosticSink* sink)
        : sharedContext(shared), diffTypeContext(shared), targetFunc(targetFunc), sink(sink)
    {
        diffTypeContext.setFunc(as<IRFunc>(targetFunc));
    }

    Result translate(IRBuilder* builder)
    {
        // TODO: This is a temporary redirect into the old solution.. once we
        // know things work, we can just move the logic into this class.

        // Do the reverse-mode translation & return the 4-tuple result.
        BackwardDiffPropagateTranscriber transcriber(sharedContext, sink);

        IRInst* bwdPrimalFunc;
        IRInst* bwdPropagateFunc;
        IRInst* bwdContextGetValFunc;
        IRInst* bwdContextType;
        transcriber._transcribeFuncImpl(
            builder,
            targetFunc,
            bwdPrimalFunc,
            bwdPropagateFunc,
            bwdContextGetValFunc,
            bwdContextType);

        // IRAssociatedInstAnnotation

        return {bwdPrimalFunc, bwdPropagateFunc, bwdContextGetValFunc, (IRType*)bwdContextType};
    }
};

// AD 2.0 version.
struct BackwardDiffTranslator
{
    // Keep track of global insts that have already been translated.
    Dictionary<IRGlobalValueWithCode*, BackwardDiffTranslationFuncContext::Result> translationCache;

    struct TranslationBaseInfo
    {
        IRFunc* funcToTranslate = nullptr;

        enum Flavor
        {
            Unknown,
            Simple,    // Direct reference to func
            Specialize // Indirect reference that requires specialization.
        } flavor = Flavor::Unknown;

        List<IRInst*> specializeOperands;
    };

    IRInst* materialize(IRBuilder* builder, TranslationBaseInfo& request, IRInst* resultInst)
    {
        if (request.flavor == TranslationBaseInfo::Flavor::Simple)
        {
            // Simple case, just return the function.
            return resultInst;
        }
        else if (request.flavor == TranslationBaseInfo::Flavor::Specialize)
        {
            // Specialization case, we need to create a new specialized function.
            // IRBuilder builder(resultInst->getModule());
            // builder.setInsertAfter(request.funcToTranslate);

            // auto outerGeneric = findOuterGeneric(resultInst);
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
                                request.specializeOperands))),
                        resultInst,
                        request.specializeOperands)));
            }
            else if (as<IRStructType>(innerVal))
            {
                return (IRType*)specializeGeneric(
                    as<IRSpecialize>(builder->emitSpecializeInst(
                        builder->getTypeKind(),
                        resultInst,
                        request.specializeOperands)));
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

    IRInst* getBase(IRInst* inst)
    {
        IRInst* baseInst = nullptr;
        switch (inst->getOp())
        {
        case kIROp_BackwardDifferentiatePrimal:
        case kIROp_BackwardDifferentiatePropagate:
        case kIROp_BackwardContextGetPrimalVal:
        case kIROp_BackwardDiffIntermediateContextType:
            baseInst = inst->getOperand(0);
            break;
        default:
            baseInst = nullptr;
        }
        return baseInst;
    }

    TranslationBaseInfo getTranslationBaseInfo(IRInst* inst)
    {
        auto baseInst = getBase(inst);
        if (baseInst)
        {
            if (auto funcInst = as<IRFunc>(baseInst))
            {
                // If the base is a function, we can directly reference it.
                TranslationBaseInfo requestInfo;
                requestInfo.funcToTranslate = funcInst;
                requestInfo.flavor = TranslationBaseInfo::Flavor::Simple;
                return requestInfo;
            }
            else if (auto specializeInst = as<IRSpecialize>(baseInst))
            {
                // If the base is a specialization, we need to collect the operands.
                TranslationBaseInfo requestInfo;
                IRGeneric* genericInst = cast<IRGeneric>(specializeInst->getBase());
                requestInfo.funcToTranslate = cast<IRFunc>(getGenericReturnVal(genericInst));
                for (UInt i = 0; i < specializeInst->getArgCount(); i++)
                    requestInfo.specializeOperands.add(specializeInst->getArg(i));
                requestInfo.flavor = TranslationBaseInfo::Flavor::Specialize;
                return requestInfo;
            }
            else
            {
                // Not sure what to do here..
                SLANG_UNEXPECTED(
                    "Expected a function or a specialization for backward differentiation.");
            }
        }

        return TranslationBaseInfo();
    }

    IRInst* processTranslationRequest(
        IRInst* translateInst,
        AutoDiffSharedContext* sharedContext,
        DiagnosticSink* sink)
    {
        auto baseInfo = getTranslationBaseInfo(translateInst);
        SLANG_ASSERT(baseInfo.flavor != TranslationBaseInfo::Flavor::Unknown);

        auto translationKey = baseInfo.funcToTranslate;
        // as<IRFunc>(getResolvedInstForDecorations(baseInfo.getBase()));
        if (!translationKey)
        {
            // TODO: diagnose
            SLANG_UNEXPECTED("Expected a global value with code for backward differentiation.");
        }

        auto extractRelevantGlobalVal =
            [&](BackwardDiffTranslationFuncContext::Result result) -> IRInst*
        {
            switch (translateInst->getOp())
            {
            case kIROp_BackwardDifferentiatePrimal:
                return result.bwdApplyFunc;
            case kIROp_BackwardDifferentiatePropagate:
                return result.bwdPropFunc;
            case kIROp_BackwardContextGetPrimalVal:
                return result.bwdValFunc;
            case kIROp_BackwardDiffIntermediateContextType:
                return result.bwdContextType;
            default:
                SLANG_UNEXPECTED("Unexpected backward differentiation operation.");
            }
        };

        if (translationCache.containsKey(translationKey))
        {
            // If we already have a translation for this function, return the requested value
            // and materialize any specializations if necessary.
            //
            IRBuilder builder(sharedContext->moduleInst);
            builder.setInsertAfter(translateInst);
            return materialize(
                &builder,
                baseInfo,
                extractRelevantGlobalVal(translationCache[translationKey]));
        }

        // Create a new context for the translation.
        BackwardDiffTranslationFuncContext context(
            as<IRGeneric>(translationKey) ? cast<IRFunc>(getGenericReturnVal(translationKey))
                                          : translationKey,
            sharedContext,
            sink);
        IRBuilder builder(sharedContext->moduleInst);
        builder.setInsertAfter(translationKey);

        BackwardDiffTranslationFuncContext::Result translationResult = context.translate(&builder);
        translationCache.add(translationKey, translationResult);
        builder.setInsertAfter(translateInst);
        return materialize(
            &builder,
            baseInfo,
            extractRelevantGlobalVal(translationCache[translationKey]));
    }
};

// AD 2.0 -> 1.0 translator.
struct LegacyBackwardDiffTranslationFuncContext
{
    struct Result
    {
        IRInst* bwdDiffFunc = nullptr;
    };

    // Shared context holding on to interface definitions, etc..
    AutoDiffSharedContext* sharedContext;

    // Differentiable type conformance context.
    DifferentiableTypeConformanceContext diffTypeContext;

    // The operands to use to construct the backward derivative function.
    IRInst* applyBwdFunc;
    IRInst* contextType;
    IRInst* bwdPropFunc;

    // The expected type of the backward derivative function.
    IRFuncType* bwdDiffFuncType;

    // The diagnostic sink to report errors.
    DiagnosticSink* sink;

    LegacyBackwardDiffTranslationFuncContext(
        IRInst* applyBwdFunc,
        IRInst* contextType,
        IRInst* bwdPropFunc,
        IRFuncType* bwdDiffFuncType,
        AutoDiffSharedContext* shared,
        DiagnosticSink* sink)
        : sharedContext(shared)
        , diffTypeContext(shared)
        , applyBwdFunc(applyBwdFunc)
        , contextType(contextType)
        , bwdPropFunc(bwdPropFunc)
        , bwdDiffFuncType(bwdDiffFuncType)
        , sink(sink)
    {
    }

    Result translate(IRBuilder* builder);
};

struct LegacyBackwardDiffTranslator
{
    // Keep track of global insts that have already been translated.
    Dictionary<IRGlobalValueWithCode*, BackwardDiffTranslationFuncContext::Result> translationCache;

    void getOperandsForTranslateInst(
        IRInst* inst,
        IRInst*& applyBwdFunc,
        IRInst*& contextType,
        IRInst*& bwdPropFunc,
        IRFuncType*& bwdDiffFuncType)
    {
        switch (inst->getOp())
        {
        case kIROp_BackwardDifferentiate:
            applyBwdFunc = inst->getOperand(0);
            contextType = inst->getOperand(1);
            bwdPropFunc = inst->getOperand(2);
            bwdDiffFuncType = cast<IRFuncType>(inst->getDataType());
            break;
        default:
            SLANG_UNEXPECTED("Unexpected backward differentiation operation.");
        }
    }

    IRInst* processTranslationRequest(
        IRInst* translateInst,
        AutoDiffSharedContext* sharedContext,
        DiagnosticSink* sink)
    {
        IRInst* applyBwdFunc;
        IRInst* contextType;
        IRInst* bwdPropFunc;
        IRFuncType* bwdDiffFuncType;
        getOperandsForTranslateInst(
            translateInst,
            applyBwdFunc,
            contextType,
            bwdPropFunc,
            bwdDiffFuncType);

        LegacyBackwardDiffTranslationFuncContext
            context(applyBwdFunc, contextType, bwdPropFunc, bwdDiffFuncType, sharedContext, sink);

        IRBuilder builder(sharedContext->moduleInst);
        // builder.setInsertAfter(as<IRGlobalValueWithCode>(applyBwdFunc));

        // This will nest the func at the right place (inside any generic contexts).
        builder.setInsertAfter(translateInst);

        LegacyBackwardDiffTranslationFuncContext::Result translationResult =
            context.translate(&builder);
        return translationResult.bwdDiffFunc;
    }
};

// AD 1.0 -> 2.0 translator.
struct LegacyToNewBackwardDiffTranslationFuncContext
{
    struct Result
    {
        IRInst* applyBwdFunc = nullptr;
        IRInst* contextType = nullptr;
        IRInst* getValFunc = nullptr;
        IRInst* bwdPropFunc = nullptr;
    };

    // Shared context holding on to interface definitions, etc..
    AutoDiffSharedContext* sharedContext;

    // Differentiable type conformance context.
    DifferentiableTypeConformanceContext diffTypeContext;

    // The operands to use to construct the backward derivative function.
    IRInst* primalFunc;
    IRInst* legacyBwdDiffFunc;

    // The expected types of the apply and propagate functions.
    // IRFuncType* applyForBwdFuncType;
    // IRFuncType* bwdPropFuncType;

    // The diagnostic sink to report errors.
    DiagnosticSink* sink;

    LegacyToNewBackwardDiffTranslationFuncContext(
        IRInst* primalFunc,
        IRInst* legacyBwdDiffFunc,
        // IRFuncType* applyForBwdFuncType,
        // IRFuncType* bwdPropFuncType,
        AutoDiffSharedContext* shared,
        DiagnosticSink* sink)
        : sharedContext(shared)
        , diffTypeContext(shared)
        , primalFunc(primalFunc)
        , legacyBwdDiffFunc(legacyBwdDiffFunc)
        //, applyForBwdFuncType(applyForBwdFuncType)
        //, bwdPropFuncType(bwdPropFuncType)
        , sink(sink)
    {
    }

    Result translate(IRBuilder* builder);
};


struct LegacyToNewBackwardDiffTranslator
{
    // Keep track of global insts that have already been translated.
    Dictionary<IRInst*, LegacyToNewBackwardDiffTranslationFuncContext::Result> translationCache;

    void getOperandsForTranslateInst(IRInst* inst, IRInst*& primalFunc, IRInst*& bwdDiffFunc)
    {
        switch (inst->getOp())
        {
        case kIROp_BackwardPrimalFromLegacyBwdDiffFunc:
        case kIROp_BackwardPropagateFromLegacyBwdDiffFunc:
        case kIROp_BackwardContextFromLegacyBwdDiffFunc:
        case kIROp_BackwardContextGetValFromLegacyBwdDiffFunc:
            primalFunc = inst->getOperand(0);
            bwdDiffFunc = inst->getOperand(1);
            break;
        default:
            SLANG_UNEXPECTED("Unexpected backward differentiation operation.");
        }
    }

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

    // TODO: Merge the
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
                else
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
                // Not sure what to do here..
                SLANG_UNEXPECTED(
                    "Expected a function or a specialization for backward differentiation.");
            }
        }
    }

    TranslationBaseInfo getTranslationBaseInfo(IRInst* inst)
    {
        IRInst* primalFunc = nullptr;
        IRInst* bwdDiffFunc = nullptr;
        getOperandsForTranslateInst(inst, primalFunc, bwdDiffFunc);

        TranslationBaseInfo info;
        bool valid = true;
        valid &= addOperand(info, primalFunc);
        valid &= addOperand(info, bwdDiffFunc);

        if (!valid)
            return TranslationBaseInfo();

        return info;
    }

    /*IRInst* getCacheKeyForTranslateInst(IRInst* translateInst)
    {
        switch (translateInst->getOp())
        {
        case kIROp_BackwardPrimalFromLegacyBwdDiffFunc:
        case kIROp_BackwardPropagateFromLegacyBwdDiffFunc:
        case kIROp_BackwardContextFromLegacyBwdDiffFunc:
        case kIROp_BackwardContextGetValFromLegacyBwdDiffFunc:
            return translateInst->getOperand(0);
        default:
            SLANG_UNEXPECTED("Unexpected backward differentiation operation.");
            return nullptr;
        }
    }*/

    IRInst* processTranslationRequest(
        IRInst* translateInst,
        AutoDiffSharedContext* sharedContext,
        DiagnosticSink* sink)
    {
        // IRInst* key = getCacheKeyForTranslateInst(translateInst);
        auto baseInfo = getTranslationBaseInfo(translateInst);

        IRBuilder builder(sharedContext->moduleInst);
        auto instKey = builder.emitIntrinsicInst(
            builder.getTypeKind(),
            kIROp_BackwardFromLegacyBwdDiffFunc,
            baseInfo.operands.getCount(),
            baseInfo.operands.getBuffer());

        auto extractRelevantGlobalVal =
            [&](LegacyToNewBackwardDiffTranslationFuncContext::Result translationResult) -> IRInst*
        {
            switch (translateInst->getOp())
            {
            case kIROp_BackwardPrimalFromLegacyBwdDiffFunc:
                return translationResult.applyBwdFunc;
            case kIROp_BackwardPropagateFromLegacyBwdDiffFunc:
                return translationResult.bwdPropFunc;
            case kIROp_BackwardContextGetValFromLegacyBwdDiffFunc:
                return translationResult.getValFunc;
            case kIROp_BackwardContextFromLegacyBwdDiffFunc:
                return translationResult.contextType;
            default:
                SLANG_UNEXPECTED("Unexpected AD 1 -> AD 2 translation operation.");
            }
        };

        if (translationCache.containsKey(instKey))
        {
            // If we already have a translation for this function, return the requested value.
            return materialize(
                &builder,
                baseInfo,
                extractRelevantGlobalVal(translationCache[instKey]));
        }

        IRInst* primalFunc;
        IRInst* legacyBwdDiffFunc;
        getOperandsForTranslateInst(translateInst, primalFunc, legacyBwdDiffFunc);

        // TODO: Lower the required function types from the front-end as operands.

        // Create a new context for the translation.
        LegacyToNewBackwardDiffTranslationFuncContext context(
            primalFunc,
            legacyBwdDiffFunc,
            /*applyForBwdFuncType,
            bwdPropFuncType,*/
            sharedContext,
            sink);

        builder.setInsertBefore(translateInst);

        LegacyToNewBackwardDiffTranslationFuncContext::Result translationResult =
            context.translate(&builder);
        translationCache.add(instKey, translationResult);

        return materialize(&builder, baseInfo, extractRelevantGlobalVal(translationResult));
    }
};

} // namespace Slang
