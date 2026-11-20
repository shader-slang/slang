// slang-ir-late-require-capability.h
#pragma once

#include "slang.h"

namespace Slang
{
struct CodeGenContext;
class DiagnosticSink;
struct IRModule;

/// Validate and eliminate late capability requirements and deferred enum conversions. Diagnose
/// missing capabilities as warnings or errors depending on whether restrictive capability checks
/// are enabled (-restrictive-capability-check).
void processLateRequireCapabilityInsts(
    IRModule* module,
    CodeGenContext* codeGenContext,
    DiagnosticSink* sink);

} // namespace Slang
