// Test that `constexpr` on a function parameter does NOT emit the E31227
// "constexpr on variable declarations is not" warning. `constexpr` on parameters is a
// supported Slang feature meaning "must be compile-time constant at call site".

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv

// CHECK-NOT: constexpr on variable declarations is not

void foo(constexpr int offset) {}

[numthreads(1, 1, 1)]
void computeMain()
{
    foo(2);
}
