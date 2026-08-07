// Boundary companion to builtin-operator-user-overload-error-11877.slang, for
// https://github.com/shader-slang/slang/issues/11877
//
// The declaration-site rejection (`checkOverloadedBuiltinOperatorDecl` in slang-check-decl.cpp)
// must reject *exactly* the overloads the builtin-operator fast path (`convertToBuiltinArithmeticOp`
// in slang-check-expr.cpp) shadows -- no more. An overload the fast path leaves to normal overload
// resolution is honored, so it must compile without the E30073 error. This test pins the boundary:
//
//   - Ineligible element type for the operator: the fast path only rewrites an operator when the
//     element type is eligible for its family, so `+`/`<`/`&` on `bool` (bool is neither integer nor
//     floating-point) and unary `!` on a non-`bool` operand fall through to resolution.
//     (`bool operator==` / `bool operator!` ARE fast-pathed and are rejected -- see the error test.)
//   - Mismatched operand types: the fast path defers a shift with unequal operand types and any
//     arithmetic pair with no common broadcast type, so `int operator<<(int, uint)` and
//     `float2 operator+(float2, float3)` are honored. (`float2 + float3` has no builtin form at all.)
//
// This compiles and runs on the CPU; the CHECK simply confirms the module type-checks and the
// entry point executes (i.e. no declaration-site error was emitted).
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

bool operator+(bool a, bool b)
{
    return a;
}
bool operator<(bool a, bool b)
{
    return a;
}
bool operator&(bool a, bool b)
{
    return a;
}
int operator!(int a)
{
    return a;
}
float3 operator!(float3 a)
{
    return a;
}
int operator<<(int a, uint b)
{
    return a;
}
float2 operator+(float2 a, float3 b)
{
    return a;
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = 42;
    // CHECK: 42
}
