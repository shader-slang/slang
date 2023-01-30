// slang-ir-autodiff-fwd.h
#pragma once

#include "slang-ir-autodiff-transcriber-base.h"

namespace Slang
{

struct ForwardDiffTranscriber : AutoDiffTranscriberBase
{
    ForwardDiffTranscriber(AutoDiffSharedContext* shared, SharedIRBuilder* inSharedBuilder, DiagnosticSink* inSink)
        : AutoDiffTranscriberBase(shared, inSharedBuilder, inSink)
    {
    }

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

    InstPair transcribeLoad(IRBuilder* builder, IRLoad* origLoad);

    InstPair transcribeStore(IRBuilder* builder, IRStore* origStore);

    // Since int/float literals are sometimes nested inside an IRConstructor
    // instruction, we check to make sure that the nested instr is a constant
    // and then return nullptr. Literals do not need to be differentiated.
    //
    InstPair transcribeConstruct(IRBuilder* builder, IRInst* origConstruct);

    // Differentiating a call instruction here is primarily about generating
    // an appropriate call list based on whichever parameters have differentials 
    // in the current transcription context.
    // 
    InstPair transcribeCall(IRBuilder* builder, IRCall* origCall);

    InstPair transcribeSwizzle(IRBuilder* builder, IRSwizzle* origSwizzle);

    InstPair transcribeByPassthrough(IRBuilder* builder, IRInst* origInst);

    InstPair transcribeControlFlow(IRBuilder* builder, IRInst* origInst);

    InstPair transcribeConst(IRBuilder* builder, IRInst* origInst);

    IRInst* findInterfaceRequirement(IRInterfaceType* type, IRInst* key);

    InstPair transcribeSpecialize(IRBuilder* builder, IRSpecialize* origSpecialize);

    InstPair transcribeFieldExtract(IRBuilder* builder, IRInst* originalInst);

    InstPair transcribeGetElement(IRBuilder* builder, IRInst* origGetElementPtr);

    InstPair transcribeUpdateElement(IRBuilder* builder, IRInst* originalInst);

    InstPair transcribeLoop(IRBuilder* builder, IRLoop* origLoop);

    InstPair transcribeIfElse(IRBuilder* builder, IRIfElse* origIfElse);

    InstPair transcribeSwitch(IRBuilder* builder, IRSwitch* origSwitch);

    InstPair transcribeMakeDifferentialPair(IRBuilder* builder, IRMakeDifferentialPair* origInst);

    InstPair transcribeDifferentialPairGetElement(IRBuilder* builder, IRInst* origInst);

    InstPair transcribeSingleOperandInst(IRBuilder* builder, IRInst* origInst);

    InstPair transcribeWrapExistential(IRBuilder* builder, IRInst* origInst);

    virtual IRFuncType* differentiateFunctionType(IRBuilder* builder, IRInst* func, IRFuncType* funcType) override;

    // Transcribe a function definition.
    InstPair transcribeFunc(IRBuilder* inBuilder, IRFunc* primalFunc, IRFunc* diffFunc);

    // Transcribe a generic definition
    InstPair transcribeGeneric(IRBuilder* inBuilder, IRGeneric* origGeneric);

    // Transcribe a function without marking the result as a decoration.
    IRFunc* transcribeFuncHeaderImpl(IRBuilder* inBuilder, IRFunc* origFunc);

    // Create an empty func to represent the transcribed func of `origFunc`.
    virtual InstPair transcribeFuncHeader(IRBuilder* inBuilder, IRFunc* origFunc) override;

    virtual InstPair transcribeInstImpl(IRBuilder* builder, IRInst* origInst) override;

    virtual IROp getInterfaceRequirementDerivativeDecorationOp() override
    {
        return kIROp_ForwardDerivativeDecoration;
    }

};

}
