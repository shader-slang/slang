// Test that `constexpr` on a function parameter does NOT emit the E31227
// "constexpr on variable declarations is not" warning. `constexpr` on parameters is a
// supported Slang feature meaning "must be compile-time constant at call site".
// Exhaustive mode with zero annotations: any diagnostic (including E31227)
// would cause this test to fail.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv

void foo(constexpr int offset) {}

[numthreads(1, 1, 1)]
void computeMain()
{
    foo(2);
}
