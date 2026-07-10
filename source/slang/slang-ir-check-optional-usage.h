// slang-ir-check-optional-usage.h
#pragma once

namespace Slang
{
class DiagnosticSink;
struct IRModule;

// Diagnose invalid Optional usage; runNonEssentialValidation gates known-none value access.
void checkForInvalidOptionalUsage(
    IRModule* module,
    DiagnosticSink* sink,
    bool runNonEssentialValidation);
} // namespace Slang
