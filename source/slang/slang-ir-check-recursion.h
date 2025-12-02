#pragma once

namespace Slang
{
struct IRModule;
class DiagnosticSink;
class TargetRequest;

void checkForRecursiveTypes(IRModule* module, DiagnosticSink* sink);

void checkForRecursiveFunctions(IRModule* module, TargetRequest* target, DiagnosticSink* sink);

} // namespace Slang
