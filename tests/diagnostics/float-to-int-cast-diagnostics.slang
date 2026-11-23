//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// An explicit cast of a floating-point literal to an integer type is not folded when the value,
// truncated toward zero, does not fit in the destination type. C, C++ and SPIR-V leave such a
// conversion undefined, and GPUs commonly saturate it, so the result is target-defined. The
// front end reports a warning at the cast, and an expression that needs the value as a
// constant, such as a `case` label, then fails.
// `tests/language-feature/const-eval/float-to-int-cast-fold.slang` covers the values that fit.

void f(int x)
{
    switch (x) { case 0: break; case (int)(int8_t)300.0f: break; }
//CHECK:                                  ^ warning E30036
//CHECK:                                  ^ converting 300.0 to 'int8_t' is target-defined.
//CHECK:                             ^ expression does not evaluate to a compile-time constant
    switch (x) { case 0: break; case (int)(uint)-1.0f: break; }
//CHECK:                                  ^ warning E30036
//CHECK:                                  ^ converting -1.0 to 'uint' is target-defined.
//CHECK:                             ^ expression does not evaluate to a compile-time constant
    switch (x) { case 0: break; case (int)(int64_t)9223372036854775808.0: break; }
//CHECK:                                  ^ warning E30036
//CHECK:                                  ^ converting 9223372036854775808.0 to 'int64_t' is target-defined.
//CHECK:                             ^ expression does not evaluate to a compile-time constant
    switch (x) { case 0: break; case (int)(uint64_t)18446744073709551616.0: break; }
//CHECK:                                  ^ warning E30036
//CHECK:                                  ^ converting 18446744073709551616.0 to 'uint64_t' is target-defined.
//CHECK:                             ^ expression does not evaluate to a compile-time constant
    switch (x) { case 0: break; case (int)1e300l: break; }
//CHECK:                             ^ warning E30036
//CHECK:                             ^ converting 1e+300 to 'int' is target-defined.
//CHECK:                             ^ expression does not evaluate to a compile-time constant
}
