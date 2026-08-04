// Test that `constexpr const` (or `const constexpr`) compiles without a
// spurious duplicate-modifier error (E31202). The `constexpr` is dropped and
// E31227 is emitted exactly once; the explicit `const` remains.
// Exhaustive mode enforces exactly-once: a second E31227 or any E31202 would
// be an unannotated diagnostic and cause failure. The span entry ("constexpr
// is treated as const") is the second annotation here.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv

//CHECK: constexpr on variable declarations is not a supported Slang feature
//CHECK: constexpr is treated as const

static constexpr const uint N = 4;

[numthreads(1, 1, 1)]
void computeMain() {}
