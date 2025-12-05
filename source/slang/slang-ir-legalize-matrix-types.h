#pragma once

namespace Slang
{

struct IRModule;
class DiagnosticSink;
class TargetProgram;

// Lower int/uint/bool matrix types to arrays for SPIRV, WGSL, GLSL, and Metal targets
void legalizeMatrixTypes(IRModule* module, TargetProgram* targetProgram, DiagnosticSink* sink);

} // namespace Slang