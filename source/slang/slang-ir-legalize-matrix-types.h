#pragma once

namespace Slang
{

struct IRMatrixType;
struct IRModule;
class DiagnosticSink;
class TargetProgram;
class TargetRequest;

/// Return whether `legalizeMatrixTypes` lowers `matrixType` to an array of row vectors on the
/// target: int/uint/bool matrices on SPIR-V, GLSL, WGSL and Metal, which have no matrix type for
/// those element types, and every matrix on LLVM.
bool isMatrixTypeLoweredToArray(TargetRequest* targetRequest, IRMatrixType* matrixType);

/// Lower every matrix for which `isMatrixTypeLoweredToArray` holds to an array of row vectors.
/// Buffer element lowering must run first, because it stores a matrix with its layout.
void legalizeMatrixTypes(IRModule* module, TargetProgram* targetProgram, DiagnosticSink* sink);

} // namespace Slang