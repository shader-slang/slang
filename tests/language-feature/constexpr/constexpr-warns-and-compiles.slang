// Test that `constexpr` emits a warning (unsupported feature) but compiles
// correctly by falling back to `const` semantics. Regression test for #10940.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv

//CHECK: constexpr on variable declarations is not a supported Slang feature

static constexpr uint N = 4;

[numthreads(1, 1, 1)]
void computeMain() {}
