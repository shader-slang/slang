#pragma once

namespace Slang
{
struct IRModule;
class DiagnosticSink;

void checkForRecursiveTypes(IRModule* module, DiagnosticSink* sink);
} // namespace Slang
