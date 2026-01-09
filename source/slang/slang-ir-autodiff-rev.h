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

    // References to other passes that for reverse-mode transcription.
    DiffTransposePass* diffTransposePass;
    DiffPropagationPass* diffPropagationPass;
    DiffUnzipPass* diffUnzipPass;

    // Allocate space for the passes.
    DiffTransposePass diffTransposePassStorage;
    DiffPropagationPass diffPropagationPassStorage;
    DiffUnzipPass diffUnzipPassStorage;

    BackwardDiffTranscriberBase(AutoDiffSharedContext* shared, DiagnosticSink* inSink)
        : AutoDiffTranscriberBase(shared, inSink)
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

    IRType* transcribeParamTypeForPrimalFunc(IRBuilder* builder, IRType* paramType);

    // Puts parameters into their own block.
    void makeParameterBlock(IRBuilder* inBuilder, IRFunc* func);

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

    SlangResult prepareFuncForBackwardDiff(IRFunc* func);

    IRFunc* generateNewForwardDerivativeForFunc(
        IRBuilder* builder,
        IRFunc* originalFunc,
        IRFunc* targetFunc);

    IRFunc* generateTrivialForwardDerivativeForFunc(
        IRBuilder* builder,
        IRFunc* originalFunc,
        IRFunc* targetFunc);

    // New interface for AD 2.0
    void _transcribeFuncImpl(
        IRBuilder* builder,
        IRFunc* targetFunc,
        IRInst*& applyFunc,
        IRInst*& propagateFunc,
        IRInst*& contextGetValFunc,
        IRInst*& contextType,
        bool isTrivial);
};

struct BackwardDiffPropagateTranscriber : BackwardDiffTranscriberBase
{
    BackwardDiffPropagateTranscriber(AutoDiffSharedContext* shared, DiagnosticSink* inSink)
        : BackwardDiffTranscriberBase(shared, inSink)
    {
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

    // Whether the target function is trivial (zero derivatives).
    bool isTrivial = false;

    // The diagnostic sink to report errors.
    DiagnosticSink* sink;

    BackwardDiffTranslationFuncContext(
        IRFunc* targetFunc,
        bool isTrivial,
        AutoDiffSharedContext* shared,
        DiagnosticSink* sink)
        : sharedContext(shared)
        , diffTypeContext(shared)
        , targetFunc(targetFunc)
        , sink(sink)
        , isTrivial(isTrivial)
    {
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
            bwdContextType,
            isTrivial);

        // IRAssociatedInstAnnotation

        return {bwdPrimalFunc, bwdPropagateFunc, bwdContextGetValFunc, (IRType*)bwdContextType};
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

IRInst* maybeTranslateLegacyToNewBackwardDerivative(
    AutoDiffSharedContext* sharedContext,
    DiagnosticSink* sink,
    IRBackwardFromLegacyBwdDiffFunc* translateInst);

IRInst* maybeTranslateLegacyBackwardDerivative(
    AutoDiffSharedContext* sharedContext,
    DiagnosticSink* sink,
    IRLegacyBackwardDifferentiate* translateInst);

IRInst* maybeTranslateBackwardDerivative(
    AutoDiffSharedContext* sharedContext,
    DiagnosticSink* sink,
    IRBackwardDifferentiate* translateInst);

IRInst* maybeTranslateTrivialBackwardDerivative(
    AutoDiffSharedContext* sharedContext,
    DiagnosticSink* sink,
    IRTrivialBackwardDifferentiate* translateInst);

} // namespace Slang
