#pragma once

namespace Slang
{
class DiagnosticSink;
struct IRModule;

void legalizeAtomicOperations(DiagnosticSink * sink, IRModule* module);
} // namespace Slang
