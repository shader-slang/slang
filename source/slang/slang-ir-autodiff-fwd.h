// slang-ir-autodiff-fwd.h
#pragma once

#include "slang-ir-autodiff-transcriber-base.h"

namespace Slang
{

struct ForwardDiffTranscriber : AutoDiffTranscriberBase
{
    // Pending values to write back to inout params at the end of the current function.
    OrderedDictionary<IRInst*, InstPair> mapInOutParamToWriteBackValue;

    // Signals the transcriber to use an IRForwardDifferentiateTrivial(func)
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

    InstPair transcribeMakeDifferentialPair(
        IRBuilder* builder,
        IRMakeDifferentialPairUserCode* origInst);

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

    // The diagnostic sink to report errors.
    DiagnosticSink* sink;

    ForwardDiffTranslationFuncContext(
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
        ForwardDiffTranscriber transcriber(sharedContext, sink);

        IRInst* fwdDiffFunc;
        transcriber._transcribeFuncImpl(builder, targetFunc, fwdDiffFunc);

        return {fwdDiffFunc};
    }
};

// AD 2.0 version.
struct ForwardDiffTranslator
{
    // Keep track of global insts that have already been translated.
    Dictionary<IRGlobalValueWithCode*, ForwardDiffTranslationFuncContext::Result> translationCache;

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
            SLANG_UNEXPECTED("Expected a global value with code for forward differentiation.");
        }

        if (translationCache.containsKey(globalValToTranslate))
        {
            // If we already have a translation for this function, return the requested value.
            return translationCache[globalValToTranslate].fwdDiffFunc;
        }

        // Create a new context for the translation.
        ForwardDiffTranslationFuncContext context(globalValToTranslate, sharedContext, sink);
        IRBuilder builder(sharedContext->moduleInst);
        builder.setInsertAfter(globalValToTranslate);

        // Translate, cache and return the result.
        ForwardDiffTranslationFuncContext::Result translationResult = context.translate(&builder);
        translationCache.add(globalValToTranslate, translationResult);
        return translationResult.fwdDiffFunc;
    }
};


} // namespace Slang
