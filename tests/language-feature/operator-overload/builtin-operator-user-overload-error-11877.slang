// Regression test for https://github.com/shader-slang/slang/issues/11877
//
// Since #11493 the builtin-operator fast path (`convertToBuiltinArithmeticOp` in
// slang-check-expr.cpp) rewrites `a OP b` on builtin scalar/vector/matrix operands directly to a
// builtin `BuiltinOperatorExpr` before overload resolution runs, so a user overload of a builtin
// operator on those operand types was silently never called -- the user got the builtin
// component-wise multiply, with no diagnostic (issue #11877).
//
// The compiler now rejects such an overload *at its declaration* (`checkOverloadedBuiltinOperatorDecl`
// in slang-check-decl.cpp), pointing the user at the GLSL operator semantics that give a GLSL-style
// matrix product. Front-end diagnostic, target-independent.
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

// A user overload of a builtin operator whose operands are ALL builtin scalar/vector/matrix types
// is the rejected case -- the exact shape from the issue.
float4x4 operator*(float4x4 m0, float4x4 m1)
{
    return mul(m1, m0);
}
//CHECK: cannot overload builtin operator '*'
//CHECK: -allow-glsl

// The rejection covers every operator the fast path rewrites, on scalar and vector operands too.
float operator+(float a, float b)
{
    return a - b;
}
//CHECK: cannot overload builtin operator '+'

float3 operator-(float3 a, float3 b)
{
    return a;
}
//CHECK: cannot overload builtin operator '-'

// Unary builtin operators are covered as well.
int operator~(int a)
{
    return a;
}
//CHECK: cannot overload builtin operator '~'

// Broadcast-compatible mixed shapes are rewritten by the fast path too (vector * scalar promotes to
// the vector shape), so they are rejected as well -- not only exact-same-type operands.
float3 operator*(float3 a, float b)
{
    return a;
}
//CHECK: cannot overload builtin operator '*'
