#pragma once

#include "slang-compiler.h"

namespace Slang
{

struct IRInst;
class DiagnosticSink;

// Legalize binary operations for Metal and WGSL targets.
//
// Ensures:
// - Shift amounts are over unsigned scalar types.
// - If one operand is a composite type (vector or matrix), and the other one is a scalar
//   type, then the scalar is converted to a composite type.
// - If 'inst' is not a shift, and if operands are integers of mixed signedness, then the
//   signed operand is converted to unsigned.
void legalizeBinaryOp(IRInst* inst, DiagnosticSink* sink, TargetProgram* targetProgram);

/// Convert scalar operands of composite binary operations into matching vectors or matrices.
///
/// HostVM bytecode derives one source size from operand zero's element width and lane count, so
/// every operand must match both properties. This pass represents implicit scalar broadcasts
/// explicitly with `makeVectorFromScalar` or `makeMatrixFromScalar`.
void legalizeScalarOperandsOfBinaryOps(IRModule* module);

// The logical binary operators such as AND and OR takes boolean types are its input.
// If they are in integer type, as an example, we need to explicitly cast to bool type.
// Also the return type from the logical operators should be a boolean type.
void legalizeLogicalAndOr(IRInst* inst, TargetProgram* targetProgram);
void legalizeLogicalAndOr(IRModule* module, TargetProgram* targetProgram);

} // namespace Slang
