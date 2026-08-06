// Regression test for #12367: a function-typed value that survives to code emission
// produced output referencing a name no prelude defines -- `Slang_FuncType<...>` for
// C++/CUDA kernels, `Func<...>` from Metal's op-name fallback -- or, for WGSL, a
// variable declaration with an empty type annotation. All four targets exited 0 and
// wrote source that cannot be compiled, with no diagnostic at all.
//
// `functype` is implemented for SPIR-V and the LLVM/CPU path, so this is a per-target
// limitation rather than a language-level one: it must be reported (E55216) instead of
// silently emitting undefined output.
//
// One directive per target: the four differ structurally. `introduceExplicitGlobalContext`
// (C++/CUDA/Metal) moves `gFn` into a `KernelContext` struct field, while WGSL keeps it as
// a global variable, so the diagnostic is reached through two different shapes.
//

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target cuda -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target cpp -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target metal -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target wgsl -entry computeMain -stage compute

int addOne(int x)
{
    return x + 1;
}

// Reading the function out of this variable is not a shape
// `isParamSuitableForSpecialization` can resolve, so the function type survives to emit.
static functype(int) -> int gFn = addOne;
//CHECK:                    ^^^ function-typed value is not supported on this target
//CHECK:                    ^^^ a function-typed value cannot be represented when generating code for the current target; it is supported only where the value can be resolved at compile time, so name the function directly at the call site (or pass it as an argument that names a function directly) instead of storing it in a variable or selecting it at runtime

int applyIt(functype(int) -> int f, int x)
{
    return f(x);
}

RWStructuredBuffer<int> outBuf;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    outBuf[0] = applyIt(gFn, 41);
}
