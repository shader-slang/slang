#pragma once

namespace Slang
{
struct IRModule;
class DiagnosticSink;

// Legalize 0-sized arrays to `void` types.
void legalizeEmptyArray(IRModule* module, DiagnosticSink* sink);

} // namespace Slang
