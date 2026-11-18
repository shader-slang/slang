//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,exit-code=0):

// Front-end constant folding of `?:`, `&&` and `||` whose first operand is a constant. The
// operand that the first operand selects gives the result, and the other operands are not
// folded, because the language does not evaluate them either. An operand that is not folded
// neither reports a fold diagnostic nor keeps the whole expression from folding.
//
// Each case is a `static const int` that is 1 when the folded expression has the expected
// value and 0 otherwise. A `switch` then uses the constant as a second case label next to
// `case 0`, which makes the front end fold it to a concrete value:
//
// - a wrong result folds to 0 and fails with "duplicate cases in switch" at the `case` line,
// - a result that does not fold to a concrete value fails with "expression does not evaluate to
//   a compile-time constant".
//
// `?:`, `&&` and `||` are never rewritten to a `BuiltinOperatorExpr`, so they are folded only by
// `SemanticsVisitor::tryConstantFoldExpr`.

// An out-of-range shift in the operand that is not selected. A division by zero is not used,
// because checking the division expression itself reports it, independently of folding.
static const int cond_true_skips_shift = ((true ? 3 : (1 << 32)) == 3) ? 1 : 0;
static const int cond_false_skips_shift = ((false ? (1 << 32) : 4) == 4) ? 1 : 0;
static const int and_false_skips_shift = ((false && ((1 << 40) == 0)) == false) ? 1 : 0;
static const int or_true_skips_shift = ((true || ((1 << 40) == 0)) == true) ? 1 : 0;

// The selected operand is still folded and diagnosed.
static const int cond_true_picks_shift = ((true ? (1 << 32) : 3) == 3) ? 1 : 0;
//CHECK:                                             ^^ target-defined value
//CHECK:                                             ^^ the value of expression 1 << 32 is target-defined.

void use()
{
    switch (0) { case 0: case cond_true_skips_shift: break; }
    switch (0) { case 0: case cond_false_skips_shift: break; }
    switch (0) { case 0: case and_false_skips_shift: break; }
    switch (0) { case 0: case or_true_skips_shift: break; }
}
