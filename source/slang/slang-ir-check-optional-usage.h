// slang-ir-check-optional-usage.h
#pragma once

namespace Slang
{
class DiagnosticSink;
struct IRModule;

// Diagnose invalid Optional usage. The opaque-payload `none` check always runs (it prevents an
// unlowerable `defaultConstruct` from reaching the backend as an internal error); the always-none
// `.value` access check is gated by runNonEssentialValidation, preserving existing behavior.
void checkForInvalidOptionalUsage(
    IRModule* module,
    DiagnosticSink* sink,
    bool runNonEssentialValidation);
} // namespace Slang
