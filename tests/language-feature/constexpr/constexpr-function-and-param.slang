// Exhaustive mode asserts the `constexpr` parameter stays silent: a spurious
// warning on it would add an unannotated row and fail.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv

//CHECK: constexpr on a function or other callable declaration is not a supported Slang feature and is ignored
//CHECK: constexpr modifier is ignored here

constexpr int LoopCount(constexpr int v) { return v; }

[numthreads(1, 1, 1)]
void computeMain()
{
    int n = LoopCount(3);
}
