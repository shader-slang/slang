// Test that `constexpr const` (or `const constexpr`) compiles without a
// spurious duplicate-modifier error (E31202). The `constexpr` is dropped and
// E31227 is emitted; the explicit `const` remains.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv

// E31227 fires (constexpr unsupported), but NOT E31202 (duplicate modifier).
//CHECK: constexpr is not a supported Slang language feature
//CHECK-NOT: duplicate modifier

static constexpr const uint N = 4;

[numthreads(1, 1, 1)]
void computeMain() {}
