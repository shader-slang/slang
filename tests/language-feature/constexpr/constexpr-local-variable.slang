// Test that `constexpr` on a local (non-static) variable emits the E31227
// warning and compiles correctly by treating the variable as `const`.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv

//CHECK: constexpr on variable declarations is not a supported Slang feature

[numthreads(1, 1, 1)]
void computeMain()
{
    constexpr int N = 5;
    int arr[N];
}
