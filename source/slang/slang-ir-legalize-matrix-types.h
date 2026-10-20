#pragma once

namespace Slang
{

struct IRModule;
class DiagnosticSink;
class TargetProgram;

// Lower matrix types the target cannot represent natively to arrays of row vectors, for SPIR-V,
// GLSL, Metal, WGSL and LLVM CPU targets (see `shouldLowerMatrixType` for the per-target rule).
void legalizeMatrixTypes(IRModule* module, TargetProgram* targetProgram, DiagnosticSink* sink);

} // namespace Slang