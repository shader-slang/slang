#pragma once

namespace Slang
{
struct IRModule;
class DiagnosticSink;

// - [x] Lower 0 length vectors to unit
// - [x] Lower 1 length vectors to scalar
// - [x] Lower too long vectors (more than 4 elements) to arrays, in `legalizeLargeVectorMatrixTypes`
void legalizeVectorTypes(IRModule* module, DiagnosticSink* sink);

} // namespace Slang
