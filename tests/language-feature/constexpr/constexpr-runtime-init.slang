// Test that `constexpr` with a runtime initializer compiles correctly after
// being rewritten to `const`. E31227 is emitted and the variable is usable
// as a runtime-initialized const — the rewrite does not impose a
// compile-time-constant requirement on the initializer.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv

//CHECK: constexpr on variable declarations is not a supported Slang feature

cbuffer Globals { uint runtimeVal; }

[numthreads(1, 1, 1)]
void computeMain()
{
    constexpr uint x = runtimeVal; // runtime initializer — treated as const
}
