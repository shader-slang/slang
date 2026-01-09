// slang-ir-autodiff-rev.h
#pragma once

#include "slang-compiler.h"
#include "slang-ir-autodiff-fwd.h"
#include "slang-ir-autodiff-transpose.h"
#include "slang-ir-autodiff-unzip.h"
#include "slang-ir-autodiff.h"
#include "slang-ir-insts.h"
#include "slang-ir-specialize.h"
#include "slang-ir.h"

namespace Slang
{

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
