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

    // Returns "d<var-name>" to use as a name hint for variables and parameters.
    // If no primal name is available, returns a blank string.
    //
    String getDiffVarName(IRInst* origVar);

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

    // Differentiating a call instruction here is primarily about generating
    // an appropriate call list based on whichever parameters have differentials
    // in the current transcription context.
    //
    InstPair transcribeCall(IRBuilder* builder, IRCall* origCall);

    InstPair transcribeSwizzle(IRBuilder* builder, IRSwizzle* origSwizzle);

    InstPair transcribeByPassthrough(IRBuilder* builder, IRInst* origInst);

    InstPair transcribeControlFlow(IRBuilder* builder, IRInst* origInst);

    InstPair transcribeConst(IRBuilder* builder, IRInst* origInst);

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

    InstPair transcribeAssociatedInstAnnotation(IRBuilder* builder, IRInst* origInst);

    IRFuncType* differentiateFunctionType(IRBuilder* builder, IRInst* func, IRFuncType* funcType);

    void generateTrivialFwdDiffFunc(IRFunc* primalFunc, IRFunc* diffFunc);

    // Transcribe a function definition.
    InstPair transcribeFunc(IRBuilder* inBuilder, IRFunc* primalFunc, IRFunc* diffFunc);

    // Transcribe a function without marking the result as a decoration.
    IRFunc* transcribeFuncHeaderImpl(IRBuilder* inBuilder, IRFunc* origFunc);

    void checkAutodiffInstDecorations(IRFunc* fwdFunc);

    SlangResult prepareFuncForForwardDiff(IRFunc* func);

    void _transcribeFuncImpl(IRBuilder* inBuilder, IRInst* origInst, IRInst*& fwdDiffFunc);

    // Create an empty func to represent the transcribed func of `origFunc`.
    InstPair transcribeFuncHeader(IRBuilder* inBuilder, IRFunc* origFunc);

    InstPair transcribeInstImpl(IRBuilder* builder, IRInst* origInst);

    InstPair transcribeFuncParam(IRBuilder* builder, IRParam* origParam, IRInst* primalType);

    void mapDifferentialInst(IRInst* origInst, IRInst* diffInst);

    void mapPrimalInst(IRInst* origInst, IRInst* primalInst);

    IRInst* lookupDiffInst(IRInst* origInst);

    IRInst* lookupDiffInst(IRInst* origInst, IRInst* defaultInst);

    bool hasDifferentialInst(IRInst* origInst);

    bool shouldUseOriginalAsPrimal(IRInst* currentParent, IRInst* origInst);

    IRInst* lookupPrimalInstImpl(IRInst* currentParent, IRInst* origInst);

    IRInst* lookupPrimalInst(IRInst* currentParent, IRInst* origInst, IRInst* defaultInst);

    IRInst* lookupPrimalInstIfExists(IRBuilder* builder, IRInst* origInst)
    {
        return lookupPrimalInst(builder->getInsertLoc().getParent(), origInst, origInst);
    }

    IRInst* lookupPrimalInst(IRBuilder* builder, IRInst* origInst)
    {
        return lookupPrimalInstImpl(builder->getInsertLoc().getParent(), origInst);
    }

    IRInst* lookupPrimalInst(IRBuilder* builder, IRInst* origInst, IRInst* defaultInst)
    {
        return lookupPrimalInst(builder->getInsertLoc().getParent(), origInst, defaultInst);
    }

    bool hasPrimalInst(IRInst* currentParent, IRInst* origInst);

    IRInst* findOrTranscribeDiffInst(IRBuilder* builder, IRInst* origInst);

    IRInst* findOrTranscribePrimalInst(IRBuilder* builder, IRInst* origInst);

    IRInst* maybeCloneForPrimalInst(IRBuilder* builder, IRInst* inst);

    InstPair transcribeNonDiffInst(IRBuilder* builder, IRInst* origInst);

    InstPair transcribeReturn(IRBuilder* builder, IRReturn* origReturn);

    InstPair transcribeParam(IRBuilder* builder, IRParam* origParam);

    InstPair transcribeBlockImpl(
        IRBuilder* builder,
        IRBlock* origBlock,
        HashSet<IRInst*>& instsToSkip);

    InstPair transcribeBlock(IRBuilder* builder, IRBlock* origBlock)
    {
        HashSet<IRInst*> ignore;
        for (auto inst = origBlock->getFirstInst(); inst; inst = inst->next)
        {
            if (inst->m_op == kIROp_Unmodified)
                ignore.add(inst);
        }

        return transcribeBlockImpl(builder, origBlock, ignore);
    }

    IRInst* transcribe(IRBuilder* builder, IRInst* origInst);

    InstPair transcribeInst(IRBuilder* builder, IRInst* origInst);

    IRType* getOrCreateDiffPairType(IRBuilder* builder, IRInst* originalType);

    IRInst* getDifferentialZeroOfType(IRBuilder* builder, IRType* primalType);
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