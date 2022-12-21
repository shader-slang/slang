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

struct BackwardDiffTranscriber : AutoDiffTranscriberBase
{
    // Map that stores the upper gradient given an IRInst*
    Dictionary<IRInst*, List<IRInst*>> upperGradients;
    Dictionary<IRInst*, IRInst*> primalToDiffPair;
    Dictionary<IRInst*, IRInst*> orginalToTranscribed;

    Dictionary<InstPair, IRInst*> differentialPairTypes;

    // References to other passes that for reverse-mode transcription.
    ForwardDiffTranscriber* fwdDiffTranscriber;
    DiffTransposePass* diffTransposePass;
    DiffPropagationPass* diffPropagationPass;
    DiffUnzipPass* diffUnzipPass;

    // Allocate space for the passes.
    DiffTransposePass               diffTransposePassStorage;
    DiffPropagationPass             diffPropagationPassStorage;
    DiffUnzipPass                   diffUnzipPassStorage;

    BackwardDiffTranscriber(AutoDiffSharedContext* shared, SharedIRBuilder* inSharedBuilder, DiagnosticSink* inSink)
        : AutoDiffTranscriberBase(shared, inSharedBuilder, inSink)
        , diffTransposePassStorage(shared)
        , diffPropagationPassStorage(shared)
        , diffUnzipPassStorage(shared)
        , diffTransposePass(&diffTransposePassStorage)
        , diffPropagationPass(&diffPropagationPassStorage)
        , diffUnzipPass(&diffUnzipPassStorage)
    { }

    // Returns "dp<var-name>" to use as a name hint for parameters.
    // If no primal name is available, returns a blank string.
    // 
    String makeDiffPairName(IRInst* origVar);

    // In differential computation, the 'default' differential value is always zero.
    // This is a consequence of differential computing being inherently linear. As a 
    // result, it's useful to have a method to generate zero literals of any (arithmetic) type.
    // The current implementation requires that types are defined linearly.
    // 
    IRInst* getDifferentialZeroOfType(IRBuilder* builder, IRType* primalType);

    InstPair transposeBlock(IRBuilder* builder, IRBlock* origBlock);

    // Puts parameters into their own block.
    void makeParameterBlock(IRBuilder* inBuilder, IRFunc* func);

    void cleanUpUnusedPrimalIntermediate(IRInst* func, IRInst* primalFunc, IRInst* intermediateType);

    // Transcribe a function definition.
    InstPair transcribeFunc(IRBuilder* builder, IRFunc* primalFunc, IRFunc* diffFunc);

    void transposeParameterBlock(IRBuilder* builder, IRFunc* diffFunc);

    IRInst* copyParam(IRBuilder* builder, IRParam* origParam);

    InstPair copyBinaryArith(IRBuilder* builder, IRInst* origArith);

    IRInst* transposeBinaryArithBackward(IRBuilder* builder, IRInst* origArith, IRInst* grad);

    InstPair copyInst(IRBuilder* builder, IRInst* origInst);

    IRInst* transposeParamBackward(IRBuilder* builder, IRInst* param, IRInst* grad);
    
    IRInst* transposeInstBackward(IRBuilder* builder, IRInst* origInst, IRInst* grad);

    InstPair transcribeSpecialize(IRBuilder* builder, IRSpecialize* origSpecialize);

    // Create an empty func to represent the transcribed func of `origFunc`.
    virtual InstPair transcribeFuncHeader(IRBuilder* inBuilder, IRFunc* origFunc) override;

    virtual IRFuncType* differentiateFunctionType(IRBuilder* builder, IRFuncType* funcType) override;

    virtual InstPair transcribeInstImpl(IRBuilder* builder, IRInst* origInst) override;

    virtual IROp getDifferentiableMethodDictionaryItemOp() override
    {
        return kIROp_ForwardDifferentiableMethodRequirementDictionaryItem;
    }

};

}
