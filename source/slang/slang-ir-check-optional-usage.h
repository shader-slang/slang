// slang-ir-check-optional-usage.h
#pragma once

namespace Slang
{
class DiagnosticSink;
struct IRModule;

void checkForInvalidOptionalUsage(IRModule* module, DiagnosticSink* sink);
} // namespace Slang
