#pragma once

namespace Slang
{
struct IRModule;
class DiagnosticSink;

void detectUninitializedResources(IRModule* module, DiagnosticSink* sink);
} // namespace Slang
