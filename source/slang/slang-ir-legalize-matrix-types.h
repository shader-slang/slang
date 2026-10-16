#pragma once

namespace Slang
{

struct IRMatrixType;
struct IRModule;
class DiagnosticSink;
class TargetProgram;
class TargetRequest;

/// Return whether `legalizeMatrixTypes` lowers any matrix on the target: SPIR-V, GLSL, WGSL, Metal
/// and LLVM.
bool targetLegalizesMatrixTypes(TargetRequest* targetRequest);

/// Return whether `legalizeMatrixTypes` lowers `matrixType` to an array of row vectors on the
/// target: matrices of 32-bit `int`, 32-bit `uint` and `bool` on SPIR-V, GLSL, WGSL and Metal,
/// and every matrix on LLVM.
bool isMatrixTypeLoweredToArray(TargetRequest* targetRequest, IRMatrixType* matrixType);

/// Lower every matrix for which `isMatrixTypeLoweredToArray` holds to an array of row vectors.
/// Buffer element lowering must run first, because it stores a matrix with its layout.
void legalizeMatrixTypes(IRModule* module, TargetProgram* targetProgram, DiagnosticSink* sink);

} // namespace Slang