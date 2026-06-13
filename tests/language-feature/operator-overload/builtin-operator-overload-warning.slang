// A user operator overload whose operands are builtin scalar/vector/matrix types is silently
// shadowed by the builtin-operator fast path (`convertToBuiltinArithmeticOp` in
// slang-check-expr.cpp), which rewrites `a + b` etc. to a `BuiltinOperatorExpr` before operator
// overload resolution runs. The overload is therefore dead code; this test verifies the warning
// fires for the builtin arithmetic / unary / vector / comparison forms.
//
// Non-exhaustive: we only assert that each declaration warns. The companion test
// `builtin-operator-overload-no-warning.slang` exhaustively verifies the non-shadowed forms stay
// silent.
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl

int operator+(int a, int b)
{
    return a;
}
//CHECK: operator overload on builtin scalar/vector/matrix operands is ignored

float3 operator*(float3 a, float3 b)
{
    return a;
}
//CHECK: operator overload on builtin scalar/vector/matrix operands is ignored

// Unary negate on a builtin scalar is also fast-pathed.
int operator-(int a)
{
    return a;
}
//CHECK: operator overload on builtin scalar/vector/matrix operands is ignored

// Equality on vector operands is fast-pathed too (the operand types, not the return type, gate
// the warning).
bool operator==(float2 a, float2 b)
{
    return false;
}
//CHECK: operator overload on builtin scalar/vector/matrix operands is ignored

void main() {}
