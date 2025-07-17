#pragma once

namespace Slang
{

struct IRModule;
class DiagnosticSink;
class TargetProgram;

// Lower int/uint/bool matrix types to arrays for SPIRV, WGSL, and GLSL targets
void legalizeMatrixTypes(TargetProgram* targetProgram, IRModule* module, DiagnosticSink* sink);

} // namespace Slang