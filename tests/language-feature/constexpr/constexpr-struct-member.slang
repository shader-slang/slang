// Test that `constexpr` on a struct member emits E31227 (falls back to const)
// and still compiles correctly.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv

//CHECK: constexpr on variable declarations is not a supported Slang feature

struct Foo
{
    static constexpr uint kCount = 8;
};

[numthreads(1, 1, 1)]
void computeMain() {}
