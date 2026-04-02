// slang-ir-any-value-inference.h
#pragma once

#include "../core/slang-common.h"
#include "slang-compiler.h"
#include "slang-ir-insts.h"
#include "slang-ir.h"

namespace Slang
{
// Detect and diagnose circular interface conformances (self-referential and
// cross-interface cycles). This runs early in the pipeline — before
// specialization — so that compilation stops before the IR reaches passes
// that cannot handle circular conformance IR.
void diagnoseCircularConformances(
    IRModule* module,
    DiagnosticSink* sink);

void inferAnyValueSizeWhereNecessary(
    IRModule* module,
    TargetProgram* targetProgram,
    DiagnosticSink* sink);
}
