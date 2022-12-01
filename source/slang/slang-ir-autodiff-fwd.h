// slang-ir-autodiff-fwd.h
#pragma once

#include "slang-ir.h"
#include "slang-ir-insts.h"
#include "slang-compiler.h"

namespace Slang
{

    template<typename P, typename D>
    struct DiffInstPair
    {
        P primal;
        D differential;
        DiffInstPair() = default;
        DiffInstPair(P primal, D differential) : primal(primal), differential(differential)
        {}
        HashCode getHashCode() const
        {
            Hasher hasher;
            hasher << primal << differential;
            return hasher.getResult();
        }
        bool operator ==(const DiffInstPair& other) const
        {
            return primal == other.primal && differential == other.differential;
        }
    };
    
    typedef DiffInstPair<IRInst*, IRInst*> InstPair;

    
struct ForwardDerivativeTranscriber
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

    List<InstPair>                          followUpFunctionsToTranscribe;

    SharedIRBuilder* sharedBuilder;
    // Witness table that `DifferentialBottom:IDifferential`.
    IRWitnessTable* differentialBottomWitness = nullptr;
    Dictionary<InstPair, IRInst*> differentialPairTypes;

    ForwardDerivativeTranscriber(AutoDiffSharedContext* shared, SharedIRBuilder* inSharedBuilder)
        : differentiableTypeConformanceContext(shared), sharedBuilder(inSharedBuilder)
    {

    }

    DiagnosticSink* getSink();

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

    IRFuncType* differentiateFunctionType(IRBuilder* builder, IRFuncType* funcType);

    // Get or construct `:IDifferentiable` conformance for a DifferentiablePair.
    IRWitnessTable* getDifferentialPairWitness(IRInst* inDiffPairType);

    IRType* getOrCreateDiffPairType(IRInst* primalType, IRInst* witness);

    IRType* getOrCreateDiffPairType(IRInst* primalType);

    IRType* differentiateType(IRBuilder* builder, IRType* origType);

    IRType* _differentiateTypeImpl(IRBuilder* builder, IRType* origType);

    IRType* tryGetDiffPairType(IRBuilder* builder, IRType* primalType);

    InstPair transcribeParam(IRBuilder* builder, IRParam* origParam);

    // Returns "d<var-name>" to use as a name hint for variables and parameters.
    // If no primal name is available, returns a blank string.
    // 
    String getJVPVarName(IRInst* origVar);

    // Returns "dp<var-name>" to use as a name hint for parameters.
    // If no primal name is available, returns a blank string.
    // 
    String makeDiffPairName(IRInst* origVar);

    InstPair transcribeVar(IRBuilder* builder, IRVar* origVar);

    InstPair transcribeBinaryArith(IRBuilder* builder, IRInst* origArith);

    InstPair transcribeBinaryLogic(IRBuilder* builder, IRInst* origLogic);

    InstPair transcribeLoad(IRBuilder* builder, IRLoad* origLoad);

    InstPair transcribeStore(IRBuilder* builder, IRStore* origStore);

    InstPair transcribeReturn(IRBuilder* builder, IRReturn* origReturn);

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

    InstPair transcribeLookupInterfaceMethod(IRBuilder* builder, IRLookupWitnessMethod* lookupInst);

    // In differential computation, the 'default' differential value is always zero.
    // This is a consequence of differential computing being inherently linear. As a 
    // result, it's useful to have a method to generate zero literals of any (arithmetic) type.
    // The current implementation requires that types are defined linearly.
    // 
    IRInst* getDifferentialZeroOfType(IRBuilder* builder, IRType* primalType);

    InstPair transcribeBlock(IRBuilder* builder, IRBlock* origBlock);

    InstPair transcribeFieldExtract(IRBuilder* builder, IRInst* originalInst);

    InstPair transcribeGetElement(IRBuilder* builder, IRInst* origGetElementPtr);

    InstPair transcribeLoop(IRBuilder* builder, IRLoop* origLoop);

    InstPair transcribeIfElse(IRBuilder* builder, IRIfElse* origIfElse);

    InstPair transcribeMakeDifferentialPair(IRBuilder* builder, IRMakeDifferentialPair* origInst);

    InstPair trascribeNonDiffInst(IRBuilder* builder, IRInst* origInst);

    InstPair transcribeDifferentialPairGetElement(IRBuilder* builder, IRInst* origInst);

    // Create an empty func to represent the transcribed func of `origFunc`.
    InstPair transcribeFuncHeader(IRBuilder* inBuilder, IRFunc* origFunc);

    // Transcribe a function definition.
    InstPair transcribeFunc(IRBuilder* inBuilder, IRFunc* primalFunc, IRFunc* diffFunc);

    // Transcribe a generic definition
    InstPair transcribeGeneric(IRBuilder* inBuilder, IRGeneric* origGeneric);

    IRInst* transcribe(IRBuilder* builder, IRInst* origInst);

    InstPair transcribeInst(IRBuilder* builder, IRInst* origInst);
};

    struct ForwardDerivativePassOptions
    {
        // Nothing for now..
    };

    bool processForwardDerivativeCalls(
        AutoDiffSharedContext*                  autodiffContext,
        DiagnosticSink*                         sink,
        ForwardDerivativePassOptions const&     options = ForwardDerivativePassOptions());

}
