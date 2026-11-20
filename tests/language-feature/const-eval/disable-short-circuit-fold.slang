//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-disable-short-circuit

// With `-disable-short-circuit`, `&&` and `||` evaluate both operands, so a constant first
// operand does not make the expression constant when the other operand is not constant. A `?:`
// with a scalar condition still evaluates only the selected operand, so it still folds.
// `short-circuit-fold.slang` covers the default, where all three operators short-circuit.

void f(int x)
{
    bool p = x > 3;
    switch (x) { case 1: break; case (true ? 2 : x): break; }
    switch (x) { case 1: break; case (true || p) ? 2 : 3: break; }
//CHECK:                                         ^ expression does not evaluate to a compile-time constant
    switch (x) { case 1: break; case (false && p) ? 2 : 3: break; }
//CHECK:                                          ^ expression does not evaluate to a compile-time constant
}
