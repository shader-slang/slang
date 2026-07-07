// Test that `constexpr` on a function declaration is silently accepted
// without emitting E31227. Unlike variable declarations, `constexpr` on
// a function is intentionally tolerated without a diagnostic — see the
// comment in checkModifier (slang-check-modifier.cpp).
// Exhaustive mode with zero annotations: any diagnostic (including E31227)
// would cause this test to fail.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv

constexpr int add(int a, int b) { return a + b; }

[numthreads(1, 1, 1)]
void computeMain()
{
    int x = add(1, 2);
}
