// slang-ir-autodiff-transcriber-base.h
#pragma once

#include "slang-compiler.h"
#include "slang-ir-autodiff.h"
#include "slang-ir-insts.h"
#include "slang-ir.h"

namespace Slang
{

struct AutoDiffTranscriberBase
{
    // Stores the mapping of arbitrary 'R-value' instructions to instructions that represent
    // their differential values.
    Dictionary<IRInst*, IRInst*> instMapD;

    // Set of insts currently being transcribed. Used to avoid infinite loops.
    HashSet<IRInst*> instsInProgress;

    // Cloning environment to hold mapping from old to new copies for the primal
    // instructions.
    IRCloneEnv cloneEnv;

    // Diagnostic sink for error messages.
    DiagnosticSink* sink;

    // Type conformance information.
    AutoDiffSharedContext* autoDiffSharedContext;

    // Builder to help with creating and accessing the 'DifferentiablePair<T>' struct
    DifferentialPairTypeBuilder* pairBuilder;

    DifferentiableTypeConformanceContext differentiableTypeConformanceContext;

    AutoDiffTranscriberBase(AutoDiffSharedContext* shared, DiagnosticSink* inSink)
        : autoDiffSharedContext(shared), differentiableTypeConformanceContext(shared), sink(inSink)
    {
        cloneEnv.squashChildrenMapping = true;
    }

    DiagnosticSink* getSink();

    // Returns "dp<var-name>" to use as a name hint for parameters.
    // If no primal name is available, returns a blank string.
    //
    String makeDiffPairName(IRInst* origVar);

    IRType* differentiateType(IRBuilder* builder, IRType* origType);

    IRType* tryGetDiffPairType(IRBuilder* builder, IRType* primalType);

    bool isExistentialType(IRType* type);

    void _markInstAsDifferential(
        IRBuilder* builder,
        IRInst* diffInst,
        IRInst* primalInst = nullptr);

    void copyOriginalDecorations(IRInst* origFunc, IRInst* diffFunc);

    void markDiffTypeInst(IRBuilder* builder, IRInst* inst, IRType* primalType);

    void markDiffPairTypeInst(IRBuilder* builder, IRInst* inst, IRType* primalType);
};

} // namespace Slang
