// Test that `constexpr` on a local (non-static) variable emits the E31227
// warning and compiles correctly by treating the variable as `const`.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv

//CHECK: constexpr is not a supported Slang language feature

[numthreads(1, 1, 1)]
void computeMain()
{
    constexpr int N = 5;
    int arr[N];
}
