// Test that `constexpr` emits a warning (unsupported feature) but compiles
// correctly by falling back to `const` semantics. Regression test for #10940.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)

//CHECK: constexpr is not a supported Slang language feature

static constexpr uint N = 4;

[numthreads(1, 1, 1)]
void computeMain() {}
