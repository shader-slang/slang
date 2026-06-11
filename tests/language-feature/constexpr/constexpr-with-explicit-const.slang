// Test that `constexpr const` (or `const constexpr`) compiles without a
// spurious duplicate-modifier error (E31202). The `constexpr` is dropped and
// E31227 is emitted exactly once; the explicit `const` remains.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv

// E31227 fires exactly once (second CHECK-NOT ensures no second occurrence),
// and NOT E31202 (duplicate modifier).
//CHECK: constexpr on variable declarations is not a supported Slang feature
//CHECK-NOT: constexpr on variable declarations is not a supported Slang feature
//CHECK-NOT: duplicate modifier

static constexpr const uint N = 4;

[numthreads(1, 1, 1)]
void computeMain() {}
