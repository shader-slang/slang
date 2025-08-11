#pragma once

namespace Slang
{

struct IRModule;
class DiagnosticSink;
class TargetProgram;

// Lower int/uint/bool matrix types to arrays for SPIRV, WGSL, GLSL, and Metal targets
void legalizeMatrixTypes(TargetProgram* targetProgram, IRModule* module, DiagnosticSink* sink);

} // namespace Slang