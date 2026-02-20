// slang-ir-autodiff-fwd.h
#pragma once

#include "slang-ir-autodiff.h"
#include "slang-ir-specialize.h"

namespace Slang
{

IRInst* maybeTranslateForwardDerivative(
    AutoDiffSharedContext* sharedContext,
    DiagnosticSink* sink,
    IRForwardDifferentiate* inst);

IRInst* maybeTranslateTrivialForwardDerivative(
    AutoDiffSharedContext* sharedContext,
    DiagnosticSink* sink,
    IRTrivialForwardDifferentiate* inst);

IRInst* maybeTranslateRawForwardDerivativeWithAnnotations(
    AutoDiffSharedContext* sharedContext,
    DiagnosticSink* sink,
    IRFunc* primalFunc);

IRInst* maybeTranslateForwardDerivativeWitness(
    AutoDiffSharedContext* sharedContext,
    DiagnosticSink* sink,
    IRSynthesizedForwardDerivativeWitnessTable* translateInst);

IRInst* maybeTranslateBackwardDerivativeWitness(
    AutoDiffSharedContext* sharedContext,
    DiagnosticSink* sink,
    IRSynthesizedBackwardDerivativeWitnessTable* translateInst);

} // namespace Slang