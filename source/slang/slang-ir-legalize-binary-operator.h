#pragma once

namespace Slang
{

struct IRInst;

// Ensures:
// - Shift amounts are over unsigned scalar types.
// - If one operand is a composite type (vector or matrix), and the other one is a scalar
//   type, then the scalar is converted to a composite type.
// - If 'inst' is not a shift, and if operands are integers of mixed signedness, then the
//   signed operand is converted to unsigned.
void legalizeBinaryOp(IRInst* inst);

} // namespace Slang
