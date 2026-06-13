// Companion to `builtin-operator-overload-warning.slang`. These operator overloads are NOT
// shadowed by the builtin-operator fast path (`convertToBuiltinArithmeticOp`): the fast path
// leaves each to normal overload resolution, so the user overload is still reachable and must
// NOT be flagged as ignored. Exhaustive mode (no `non-exhaustive`): any spurious diagnostic --
// including a wrongly-emitted shadowing warning -- fails the test.
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl

struct S
{
    int v;
}

// User operand type: the fast path is gated on builtin operands, so this resolves normally.
S operator+(S a, int b)
{
    return a;
}

// Bitwise on floating-point operands is not a valid builtin operation, so the fast path declines
// and this overload still resolves.
float operator&(float a, float b)
{
    return a;
}

// Unary `+` has no builtin fast-path form (only unary `-`/`!`/`~` do), so it is not shadowed.
int operator+(int a)
{
    return a;
}

// matrix * vector is not a builtin component-wise operation (no common broadcast type), so the
// fast path declines and this overload is reachable.
float3 operator*(float3x3 m, float3 v)
{
    return v;
}

void main() {}
