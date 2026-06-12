#pragma once

namespace Slang
{
struct IRModule;
class DiagnosticSink;
class TargetRequest;

void checkUnsupportedCoherentMemoryQualifier(
    IRModule* module,
    TargetRequest* target,
    DiagnosticSink* sink);
void checkUnsupportedInst(IRModule* module, TargetRequest* target, DiagnosticSink* sink);
} // namespace Slang
