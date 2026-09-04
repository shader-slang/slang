#pragma once

namespace Slang
{
struct IRModule;
class DiagnosticSink;
class TargetProgram;

void checkUnsupportedInst(IRModule* module, TargetProgram* targetProgram, DiagnosticSink* sink);
} // namespace Slang
