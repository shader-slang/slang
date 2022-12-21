// slang-ir-autodiff-transcriber-base.h
#pragma once

#include "slang-ir.h"
#include "slang-ir-insts.h"
#include "slang-compiler.h"
#include "slang-ir-autodiff.h"

namespace Slang
{

struct AutoDiffTranscriberBase
{
    // Stores the mapping of arbitrary 'R-value' instructions to instructions that represent
    // their differential values.
    Dictionary<IRInst*, IRInst*>            instMapD;

    // Set of insts currently being transcribed. Used to avoid infinite loops.
    HashSet<IRInst*>                        instsInProgress;

    // Cloning environment to hold mapping from old to new copies for the primal
    // instructions.
    IRCloneEnv                              cloneEnv;

    // Diagnostic sink for error messages.
    DiagnosticSink*                         sink;

    // Type conformance information.
    AutoDiffSharedContext*                  autoDiffSharedContext;

    // Builder to help with creating and accessing the 'DifferentiablePair<T>' struct
    DifferentialPairTypeBuilder*            pairBuilder;

    DifferentiableTypeConformanceContext    differentiableTypeConformanceContext;

    SharedIRBuilder* sharedBuilder;

    Dictionary<InstPair, IRInst*> differentialPairTypes;

    AutoDiffTranscriberBase(AutoDiffSharedContext* shared, SharedIRBuilder* inSharedBuilder, DiagnosticSink* inSink)
        : autoDiffSharedContext(shared)
        , differentiableTypeConformanceContext(shared)
        , sharedBuilder(inSharedBuilder)
        , sink(inSink)
    {

    }

    DiagnosticSink* getSink();

    // Returns "dp<var-name>" to use as a name hint for parameters.
    // If no primal name is available, returns a blank string.
    // 
    String makeDiffPairName(IRInst* origVar);

    void mapDifferentialInst(IRInst* origInst, IRInst* diffInst);

    void mapPrimalInst(IRInst* origInst, IRInst* primalInst);

    IRInst* lookupDiffInst(IRInst* origInst);

    IRInst* lookupDiffInst(IRInst* origInst, IRInst* defaultInst);

    bool hasDifferentialInst(IRInst* origInst);

    bool shouldUseOriginalAsPrimal(IRInst* origInst);

    IRInst* lookupPrimalInst(IRInst* origInst);

    IRInst* lookupPrimalInst(IRInst* origInst, IRInst* defaultInst);

    bool hasPrimalInst(IRInst* origInst);

    IRInst* findOrTranscribeDiffInst(IRBuilder* builder, IRInst* origInst);

    IRInst* findOrTranscribePrimalInst(IRBuilder* builder, IRInst* origInst);

    IRInst* maybeCloneForPrimalInst(IRBuilder* builder, IRInst* inst);

    List<IRInterfaceRequirementEntry*> AutoDiffTranscriberBase::
        findDifferentiableInterfaceLookupPath(IRInterfaceType* idiffType, IRInterfaceType* type);

    InstPair transcribeExtractExistentialWitnessTable(IRBuilder* builder, IRInst* origInst);

    // Get or construct `:IDifferentiable` conformance for a DifferentiablePair.
    IRWitnessTable* getDifferentialPairWitness(IRInst* inDiffPairType);

    IRType* getOrCreateDiffPairType(IRInst* primalType, IRInst* witness);

    IRType* getOrCreateDiffPairType(IRInst* primalType);

    IRType* differentiateType(IRBuilder* builder, IRType* origType);

    IRType* differentiateExtractExistentialType(IRBuilder* builder, IRExtractExistentialType* origType, IRInst*& witnessTable);

    IRType* tryGetDiffPairType(IRBuilder* builder, IRType* primalType);

    IRInst* findInterfaceRequirement(IRInterfaceType* type, IRInst* key);

    IRInst* getDifferentialZeroOfType(IRBuilder* builder, IRType* primalType);

    InstPair trascribeNonDiffInst(IRBuilder* builder, IRInst* origInst);

    InstPair transcribeReturn(IRBuilder* builder, IRReturn* origReturn);

    InstPair transcribeParam(IRBuilder* builder, IRParam* origParam);

    InstPair transcribeLookupInterfaceMethod(IRBuilder* builder, IRLookupWitnessMethod* lookupInst);

    InstPair transcribeBlock(IRBuilder* builder, IRBlock* origBlock);

    // Transcribe a generic definition
    InstPair transcribeGeneric(IRBuilder* inBuilder, IRGeneric* origGeneric);

    IRInst* transcribe(IRBuilder* builder, IRInst* origInst);
    
    InstPair transcribeInst(IRBuilder* builder, IRInst* origInst);

    IRType* _differentiateTypeImpl(IRBuilder* builder, IRType* origType);

    virtual IRFuncType* differentiateFunctionType(IRBuilder* builder, IRFuncType* funcType) = 0;

    // Create an empty func to represent the transcribed func of `origFunc`.
    virtual InstPair transcribeFuncHeader(IRBuilder* inBuilder, IRFunc* origFunc) = 0;

    virtual InstPair transcribeInstImpl(IRBuilder* builder, IRInst* origInst) = 0;

    virtual IROp getDifferentiableMethodDictionaryItemOp() = 0;
};

}
