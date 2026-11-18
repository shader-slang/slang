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

// Splat the scalar operand of a binary operation whose other operand is a composite type
// (vector or matrix) to that composite shape. For a shift, the splat keeps the scalar's own
// element type: the shift overloads type the result independently of the other operand's
// element type (`vector<L,N> operator>>(L left, vector<R,N> right)` yields vector<L,N>), so
// retyping the scalar would change the result. For any other operator the splat takes the
// composite operand's own type, which matches the scalar's element type because the checker
// unifies them before lowering. Callers: `legalizeBinaryOp` (Metal and WGSL, every binary
// operator) and `legalizeShiftOperandsForGLSL` (every Lsh/Rsh with a scalar operand, in
// either position — see its declaration for the GLSL rule).
void legalizeScalarOperandsToMatchComposite(IRInst* inst);

// The logical binary operators such as AND and OR takes boolean types are its input.
// If they are in integer type, as an example, we need to explicitly cast to bool type.
// Also the return type from the logical operators should be a boolean type.
void legalizeLogicalAndOr(IRInst* inst, TargetProgram* targetProgram);
void legalizeLogicalAndOr(IRModule* module, TargetProgram* targetProgram);

} // namespace Slang
