// Test that bare `constexpr` at module scope (without `static`) produces
// both E31227 (constexpr→const rewrite) and E31224 (global const requires
// static). Contrast with `static constexpr uint N = 4;` which only produces
// E31227.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv

//CHECK: constexpr on variable declarations is not a supported Slang feature
//CHECK: global const requires static

constexpr uint N = 4;

[numthreads(1, 1, 1)]
void computeMain() {}
