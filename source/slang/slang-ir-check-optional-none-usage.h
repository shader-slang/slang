// slang-ir-check-optional-none-usage.h
#pragma once

namespace Slang
{
class DiagnosticSink;
struct IRModule;

void checkForOptionalNoneUsage(IRModule* module, DiagnosticSink* sink);
} // namespace Slang
