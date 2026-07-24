// Companion to builtin-operator-user-overload-glsl-11877.slang, for
// https://github.com/shader-slang/slang/issues/11877
//
// GLSL operator scope (`import glsl;` here, or `-allow-glsl`) does NOT make every builtin-operator
// overload legal -- the fast path (`convertToBuiltinArithmeticOp` in slang-check-expr.cpp) defers
// only *matrix* operators and *vector equality* to normal resolution in GLSL scope. A scalar
// operator such as `float + float` is still rewritten to the builtin, so a user `operator+(float,
// float)` is still silently shadowed there and must still be diagnosed (only the matrix companion
// test is honored under `import glsl;`). Front-end diagnostic, target-independent.
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

import glsl;

float operator+(float a, float b)
{
    return a - b;
}
//CHECK: cannot overload builtin operator '+'
