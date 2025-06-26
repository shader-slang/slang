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

        return {bwdPrimalFunc, bwdPropagateFunc, bwdContextGetValFunc, (IRType*)bwdContextType};
    }
};

// AD 2.0 version.
struct BackwardDiffTranslator
{
    // Keep track of global insts that have already been translated.
    Dictionary<IRGlobalValueWithCode*, BackwardDiffTranslationFuncContext::Result> translationCache;

    IRInst* getBaseForTranslateInst(IRInst* inst)
    {
        switch (inst->getOp())
        {
        case kIROp_BackwardDifferentiatePrimal:
        case kIROp_BackwardDifferentiatePropagate:
        case kIROp_BackwardContextGetPrimalVal:
        case kIROp_BackwardDiffIntermediateContextType:
            return inst->getOperand(0);
        default:
            return nullptr;
        }
    }

    IRInst* processTranslationRequest(
        IRInst* translateInst,
        AutoDiffSharedContext* sharedContext,
        DiagnosticSink* sink)
    {
        auto baseInst = getBaseForTranslateInst(translateInst);

        auto globalValToTranslate = as<IRFunc>(getResolvedInstForDecorations(baseInst));
        if (!globalValToTranslate)
        {
            // TODO: diagnose
            SLANG_UNEXPECTED("Expected a global value with code for backward differentiation.");
        }

        auto extractRelevantGlobalVal = [&](BackwardDiffTranslationFuncContext::Result) -> IRInst*
        {
            auto translationResult = translationCache[globalValToTranslate];
            switch (translateInst->getOp())
            {
            case kIROp_BackwardDifferentiatePrimal:
                return translationResult.bwdApplyFunc;
            case kIROp_BackwardDifferentiatePropagate:
                return translationResult.bwdPropFunc;
            case kIROp_BackwardContextGetPrimalVal:
                return translationResult.bwdValFunc;
            case kIROp_BackwardDiffIntermediateContextType:
                return translationResult.bwdContextType;
            default:
                SLANG_UNEXPECTED("Unexpected backward differentiation operation.");
            }
        };

        if (translationCache.containsKey(globalValToTranslate))
        {
            // If we already have a translation for this function, return the requested value.
            return extractRelevantGlobalVal(translationCache[globalValToTranslate]);
        }

        // Create a new context for the translation.
        BackwardDiffTranslationFuncContext context(globalValToTranslate, sharedContext, sink);
        IRBuilder builder(sharedContext->moduleInst);
        builder.setInsertAfter(globalValToTranslate);

        BackwardDiffTranslationFuncContext::Result translationResult = context.translate(&builder);
        translationCache.add(globalValToTranslate, translationResult);

        return extractRelevantGlobalVal(translationResult);
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
    IRFunc* applyBwdFunc;
    IRType* contextType;
    IRFunc* bwdPropFunc;

    // The expected type of the backward derivative function.
    IRFuncType* bwdDiffFuncType;

    // The diagnostic sink to report errors.
    DiagnosticSink* sink;

    LegacyBackwardDiffTranslationFuncContext(
        IRFunc* applyBwdFunc,
        IRType* contextType,
        IRFunc* bwdPropFunc,
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
        IRType*& contextType,
        IRFunc*& bwdPropFunc,
        IRFuncType*& bwdDiffFuncType)
    {
        switch (inst->getOp())
        {
        case kIROp_BackwardDifferentiate:
            applyBwdFunc = inst->getOperand(0);
            contextType = as<IRType>(inst->getOperand(1));
            bwdPropFunc = as<IRFunc>(inst->getOperand(2));
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
        IRType* contextType;
        IRFunc* bwdPropFunc;
        IRFuncType* bwdDiffFuncType;
        getOperandsForTranslateInst(
            translateInst,
            applyBwdFunc,
            contextType,
            bwdPropFunc,
            bwdDiffFuncType);

        LegacyBackwardDiffTranslationFuncContext context(
            as<IRFunc>(applyBwdFunc),
            contextType,
            bwdPropFunc,
            bwdDiffFuncType,
            sharedContext,
            sink);

        IRBuilder builder(sharedContext->moduleInst);
        builder.setInsertAfter(as<IRGlobalValueWithCode>(applyBwdFunc));

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
        IRInst* bwdPropFunc = nullptr;
    };

    // Shared context holding on to interface definitions, etc..
    AutoDiffSharedContext* sharedContext;

    // Differentiable type conformance context.
    DifferentiableTypeConformanceContext diffTypeContext;

    // The operands to use to construct the backward derivative function.
    IRFunc* primalFunc;
    IRFunc* legacyBwdDiffFunc;

    // The diagnostic sink to report errors.
    DiagnosticSink* sink;

    LegacyToNewBackwardDiffTranslationFuncContext(
        IRFunc* primalFunc,
        IRFunc* legacyBwdDiffFunc,
        AutoDiffSharedContext* shared,
        DiagnosticSink* sink)
        : sharedContext(shared)
        , diffTypeContext(shared)
        , primalFunc(primalFunc)
        , legacyBwdDiffFunc(legacyBwdDiffFunc)
        , sink(sink)
    {
    }

    Result translate(IRBuilder* builder);
};


struct LegacyToNewBackwardDiffTranslator
{
    // Keep track of global insts that have already been translated.
    Dictionary<IRInst*, LegacyToNewBackwardDiffTranslationFuncContext::Result> translationCache;

    void getOperandsForTranslateInst(
        IRInst* inst,
        IRInst*& primalFunc,
        IRInst*& bwdDiffFunc,
        IRFuncType*& applyForBwdFuncType,
        IRFuncType*& bwdPropFuncType)
    {
        switch (inst->getOp())
        {
        case kIROp_BackwardPrimalFromLegacyBwdDiffFunc:
        case kIROp_BackwardPropagateFromLegacyBwdDiffFunc:
        case kIROp_BackwardContextFromLegacyBwdDiffFunc:
            primalFunc = inst->getOperand(0);
            bwdDiffFunc = inst->getOperand(1);
            break;
        default:
            SLANG_UNEXPECTED("Unexpected backward differentiation operation.");
        }

        // Find the use of "inst" in IRBackwardPrimalFromLegacyBwdDiffFunc
        // and IRBackwardPropagateFromLegacyBwdDiffFunc, and copy the func types from there.
        // If they don't exist, error out.
        //
        applyForBwdFuncType = nullptr;
        bwdPropFuncType = nullptr;

        // Scan all uses of the primalFunc to find where it's used with the expected operations
        for (auto use = primalFunc->firstUse; use; use = use->nextUse)
        {
            auto user = use->getUser();
            if (user->getOp() == kIROp_BackwardPrimalFromLegacyBwdDiffFunc)
            {
                applyForBwdFuncType = as<IRFuncType>(user->getDataType());
            }
            else if (user->getOp() == kIROp_BackwardPropagateFromLegacyBwdDiffFunc)
            {
                bwdPropFuncType = as<IRFuncType>(user->getDataType());
            }
        }

        // If we still don't have both function types, this is an error
        if (!applyForBwdFuncType || !bwdPropFuncType)
        {
            SLANG_UNEXPECTED(
                "Could not determine function types for backward differentiation translation");
        }
    }

    IRInst* processTranslationRequest(
        IRInst* translateInst,
        AutoDiffSharedContext* sharedContext,
        DiagnosticSink* sink)
    {
        IRInst* primalFunc;
        IRInst* legacyBwdDiffFunc;
        getOperandsForTranslateInst(translateInst, primalFunc, legacyBwdDiffFunc);

        auto extractRelevantGlobalVal =
            [&](LegacyToNewBackwardDiffTranslationFuncContext::Result) -> IRInst*
        {
            auto translationResult = translationCache[primalFunc];
            switch (translateInst->getOp())
            {
            case kIROp_BackwardPrimalFromLegacyBwdDiffFunc:
                return translationResult.applyBwdFunc;
            case kIROp_BackwardPropagateFromLegacyBwdDiffFunc:
                return translationResult.bwdPropFunc;
            case kIROp_BackwardContextFromLegacyBwdDiffFunc:
                return translationResult.contextType;
            default:
                SLANG_UNEXPECTED("Unexpected AD 1 -> AD 2 translation operation.");
            }
        };

        if (translationCache.containsKey(primalFunc))
        {
            // If we already have a translation for this function, return the requested value.
            return extractRelevantGlobalVal(translationCache[primalFunc]);
        }

        // Create a new context for the translation.
        LegacyToNewBackwardDiffTranslationFuncContext context(
            as<IRFunc>(primalFunc),
            as<IRFunc>(legacyBwdDiffFunc),
            sharedContext,
            sink);
        IRBuilder builder(sharedContext->moduleInst);
        builder.setInsertAfter(primalFunc);

        LegacyToNewBackwardDiffTranslationFuncContext::Result translationResult =
            context.translate(&builder);
        translationCache.add(primalFunc, translationResult);

        return extractRelevantGlobalVal(translationResult);
    }
};

} // namespace Slang
