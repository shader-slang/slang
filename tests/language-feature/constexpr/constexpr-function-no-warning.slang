// Test that `constexpr` on a function declaration is silently accepted
// without emitting E31227. Unlike variable declarations, `constexpr` on
// a function is intentionally tolerated without a diagnostic — see the
// comment in checkModifier (slang-check-modifier.cpp).

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv

// CHECK-NOT: constexpr is not a supported Slang language feature

constexpr int add(int a, int b) { return a + b; }

[numthreads(1, 1, 1)]
void computeMain()
{
    int x = add(1, 2);
}
