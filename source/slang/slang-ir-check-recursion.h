#pragma once

namespace Slang
{
struct IRModule;
struct IRType;
class DiagnosticSink;
class TargetRequest;


void checkForRecursiveTypes(IRModule* module, DiagnosticSink* sink);

void checkForRecursiveFunctions(IRModule* module, TargetRequest* target, DiagnosticSink* sink);

bool isTypeRecursive(IRType* type);

} // namespace Slang
