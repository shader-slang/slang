#pragma once

namespace Slang
{

struct IRMatrixType;
struct IRModule;
class DiagnosticSink;
class TargetProgram;

// Return true if `legalizeMatrixTypes` rewrites `matrixType` into an array of vectors on the
// given target. Buffer element type lowering uses this to build the storage type of such a matrix
// before `legalizeMatrixTypes` erases the matrix layout.
bool isMatrixTypeLegalizedToVectorArray(TargetProgram* targetProgram, IRMatrixType* matrixType);

// Lower int/uint/bool matrix types to arrays for SPIRV, WGSL, GLSL, and Metal targets
void legalizeMatrixTypes(IRModule* module, TargetProgram* targetProgram, DiagnosticSink* sink);

} // namespace Slang