// slang-ir-autodiff-rev.h
#pragma once

#include "slang-ir.h"
#include "slang-ir-insts.h"
#include "slang-compiler.h"

#include "slang-ir-autodiff.h"
#include "slang-ir-autodiff-fwd.h"
#include "slang-ir-autodiff-transcriber-base.h"
#include "slang-ir-autodiff-propagate.h"
#include "slang-ir-autodiff-unzip.h"
#include "slang-ir-autodiff-transpose.h"

namespace Slang
{

struct IRReverseDerivativePassOptions
{
    // Nothing for now..
};

struct BackwardDiffTranscriberBase : AutoDiffTranscriberBase
{
    FuncBodyTranscriptionTaskType diffTaskType;

    // Map that stores the upper gradient given an IRInst*
    Dictionary<IRInst*, List<IRInst*>> upperGradients;
    Dictionary<IRInst*, IRInst*> primalToDiffPair;
    Dictionary<IRInst*, IRInst*> orginalToTranscribed;

    // References to other passes that for reverse-mode transcription.
    DiffTransposePass* diffTransposePass;
    DiffPropagationPass* diffPropagationPass;
    DiffUnzipPass* diffUnzipPass;

    // Allocate space for the passes.
    DiffTransposePass               diffTransposePassStorage;
    DiffPropagationPass             diffPropagationPassStorage;
    DiffUnzipPass                   diffUnzipPassStorage;

    BackwardDiffTranscriberBase(
        FuncBodyTranscriptionTaskType taskType,
        AutoDiffSharedContext* shared,
        SharedIRBuilder* inSharedBuilder,
        DiagnosticSink* inSink)
        : AutoDiffTranscriberBase(shared, inSharedBuilder, inSink)
        , diffTaskType(taskType)
        , diffTransposePassStorage(shared)
        , diffPropagationPassStorage(shared)
        , diffUnzipPassStorage(shared)
        , diffTransposePass(&diffTransposePassStorage)
        , diffPropagationPass(&diffPropagationPassStorage)
        , diffUnzipPass(&diffUnzipPassStorage)
    {}

    // Returns "dp<var-name>" to use as a name hint for parameters.
    // If no primal name is available, returns a blank string.
    // 
    String makeDiffPairName(IRInst* origVar);
        
    IRFuncType* differentiateFunctionTypeImpl(IRBuilder* builder, IRFuncType* funcType, IRInst* intermediateType);

    IRType* transcribeParamTypeForPrimalFunc(IRBuilder* builder, IRType* paramType);
    IRType* transcribeParamTypeForPropagateFunc(IRBuilder* builder, IRType* paramType);

    // Puts parameters into their own block.
    void makeParameterBlock(IRBuilder* inBuilder, IRFunc* func);

    // Transcribe a function definition.
    virtual InstPair transcribeFunc(IRBuilder* builder, IRFunc* primalFunc, IRFunc* diffFunc) = 0;

    // Transcribes the parameter block and returns the dOut param if exists.
    IRInst* transposeParameterBlock(IRBuilder* builder, IRFunc* diffFunc, List<IRInst*>& primalFuncSpecificParams, bool isResultDifferentiable);

    InstPair transcribeFuncParam(IRBuilder* builder, IRParam* origParam, IRInst* primalType);

    InstPair transcribeSpecialize(IRBuilder* builder, IRSpecialize* origSpecialize);

    SlangResult prepareFuncForBackwardDiff(IRFunc* func);

    IRFunc* generateNewForwardDerivativeForFunc(IRBuilder* builder, IRFunc* originalFunc, IRFunc* diffPropagateFunc);

    void transcribeFuncImpl(IRBuilder* builder, IRFunc* primalFunc, IRFunc* diffPropagateFunc);

    InstPair transcribeFuncHeaderImpl(IRBuilder* inBuilder, IRFunc* origFunc);

    void addTranscribedFuncDecoration(IRBuilder& builder, IRFunc* origFunc, IRFunc* transcribedFunc);

    virtual InstPair transcribeFuncHeader(IRBuilder* inBuilder, IRFunc* origFunc) override;

    virtual InstPair transcribeInstImpl(IRBuilder* builder, IRInst* origInst) override;

    virtual IRInst* findExistingDiffFunc(IRInst* originalFunc) = 0;
    virtual void addExistingDiffFuncDecor(IRBuilder* builder, IRInst* inst, IRInst* diffFunc) = 0;

    virtual IROp getInterfaceRequirementDerivativeDecorationOp() override
    {
        return kIROp_BackwardDerivativeDecoration;
    }
};

struct BackwardDiffPrimalTranscriber : BackwardDiffTranscriberBase
{
    BackwardDiffPrimalTranscriber(
        AutoDiffSharedContext* shared,
        SharedIRBuilder* inSharedBuilder,
        DiagnosticSink* inSink)
        : BackwardDiffTranscriberBase(
              FuncBodyTranscriptionTaskType::BackwardPrimal, shared, inSharedBuilder, inSink)
    { }

    virtual IRFuncType* differentiateFunctionType(IRBuilder* builder, IRInst* func, IRFuncType* funcType) override;
    virtual InstPair transcribeFunc(IRBuilder* builder, IRFunc* primalFunc, IRFunc* diffFunc) override;
    virtual IRInst* findExistingDiffFunc(IRInst* originalFunc) override
    {
        if (auto backDecor = originalFunc->findDecoration<IRBackwardDerivativePrimalDecoration>())
        {
            return backDecor->getBackwardDerivativePrimalFunc();
        }
        return nullptr;
    }
    virtual void addExistingDiffFuncDecor(IRBuilder* builder, IRInst* inst, IRInst* diffFunc) override
    {
        builder->addBackwardDerivativePrimalDecoration(inst, diffFunc);
    }
    virtual IROp getInterfaceRequirementDerivativeDecorationOp() override
    {
        return kIROp_BackwardDerivativePrimalDecoration;
    }
};

struct BackwardDiffPropagateTranscriber : BackwardDiffTranscriberBase
{
    BackwardDiffPropagateTranscriber(
        AutoDiffSharedContext* shared,
        SharedIRBuilder* inSharedBuilder,
        DiagnosticSink* inSink)
        : BackwardDiffTranscriberBase(
              FuncBodyTranscriptionTaskType::BackwardPropagate,
              shared,
              inSharedBuilder,
              inSink)
    { }
    void generateTrivialDiffFuncFromUserDefinedDerivative(
        IRBuilder* builder,
        IRFunc* primalFunc,
        IRFunc* diffPropFunc,
        IRUserDefinedBackwardDerivativeDecoration* udfDecor);

    virtual IRFuncType* differentiateFunctionType(IRBuilder* builder, IRInst* func, IRFuncType* funcType) override;
    virtual InstPair transcribeFunc(IRBuilder* builder, IRFunc* primalFunc, IRFunc* diffFunc) override;
    virtual IRInst* findExistingDiffFunc(IRInst* originalFunc) override
    {
        if (auto backDecor = originalFunc->findDecoration<IRBackwardDerivativePropagateDecoration>())
        {
            return backDecor->getBackwardDerivativePropagateFunc();
        }
        return nullptr;
    }
    virtual void addExistingDiffFuncDecor(IRBuilder* builder, IRInst* inst, IRInst* diffFunc) override
    {
        builder->addBackwardDerivativePropagateDecoration(inst, diffFunc);
    }
    virtual IROp getInterfaceRequirementDerivativeDecorationOp() override
    {
        return kIROp_BackwardDerivativePropagateDecoration;
    }
};

// A backward derivative function combines both primal + propagate functions and accepts no
// intermediate value input.
struct BackwardDiffTranscriber : BackwardDiffTranscriberBase
{
    BackwardDiffTranscriber(
        AutoDiffSharedContext* shared,
        SharedIRBuilder* inSharedBuilder,
        DiagnosticSink* inSink)
        : BackwardDiffTranscriberBase(
              FuncBodyTranscriptionTaskType::Backward, shared, inSharedBuilder, inSink)
    { }

    virtual IRFuncType* differentiateFunctionType(IRBuilder* builder, IRInst* func, IRFuncType* funcType) override;
    virtual InstPair transcribeFuncHeader(IRBuilder* inBuilder, IRFunc* origFunc) override;
    virtual InstPair transcribeFunc(IRBuilder* builder, IRFunc* primalFunc, IRFunc* diffFunc) override
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
        if (auto backDecor = originalFunc->findDecoration<IRUserDefinedBackwardDerivativeDecoration>())
        {
            return backDecor->getBackwardDerivativeFunc();
        }
        return nullptr;
    }
    virtual void addExistingDiffFuncDecor(IRBuilder* builder, IRInst* inst, IRInst* diffFunc) override
    {
        builder->addBackwardDerivativeDecoration(inst, diffFunc);
    }
};

}
