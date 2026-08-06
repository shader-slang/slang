// Companion to func-type-unsupported-on-kernel-target.slang, covering #12367 for a
// function-typed local variable and the parameter it is passed to.
//
// A `select` between two functions is not a shape `isParamSuitableForSpecialization`
// accepts, so the function type survives on both the local and on `applyIt`'s parameter.
// Neither is a global or a `KernelContext` field, so this is the shape that pins the
// check to function bodies as well as to module-level declarations.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target cuda -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target cpp -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target metal -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target wgsl -entry computeMain -stage compute

int addOne(int x)
{
    return x + 1;
}

int addTwo(int x)
{
    return x + 2;
}

int applyIt(functype(int) -> int f, int x)
//CHECK:                         ^ function-typed value is not supported on this target
//CHECK:                         ^ a function-typed value cannot be represented when generating code for the current target; it is supported only where the value can be resolved at compile time, so name the function directly at the call site (or pass it as an argument that names a function directly) instead of storing it in a variable or selecting it at runtime
{
    return f(x);
}

RWStructuredBuffer<int> outBuf;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    functype(int) -> int local = (tid.x > 0) ? addOne : addTwo;
//CHECK:                                     ^ function-typed value is not supported on this target
//CHECK:                                     ^ a function-typed value cannot be represented when generating code for the current target; it is supported only where the value can be resolved at compile time, so name the function directly at the call site (or pass it as an argument that names a function directly) instead of storing it in a variable or selecting it at runtime
    outBuf[0] = applyIt(local, 41);
}
