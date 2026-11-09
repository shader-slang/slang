// Companion to the other #12367 tests, covering a function-typed `groupshared` global.
//
// `groupshared` wraps the variable's type in an `IRRateQualifiedType`, which is neither an array
// nor a pointer, so looking through those alone does not reach the function type inside. The check
// asks for the value's data type, which strips the rate -- as the two neighbouring checks in the
// same pass already do.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target cuda -entry computeMain -stage compute
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
{
    return f(x);
}

RWStructuredBuffer<int> outBuf;

groupshared functype(int) -> int gFn;
//CHECK:                         ^^^ function-typed value is not supported on this target
//CHECK:                         ^^^ a function-typed value cannot be represented when generating code for the current target; it is supported only where the value can be resolved at compile time, so name the function directly at the call site (or pass it as an argument that names a function directly) instead of storing it in a variable or selecting it at runtime

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    if (tid.x == 0)
    {
        gFn = (tid.y > 0) ? addOne : addTwo;
//CHECK:                  ^ function-typed value is not supported on this target
//CHECK:                  ^ a function-typed value cannot be represented when generating code for the current target; it is supported only where the value can be resolved at compile time, so name the function directly at the call site (or pass it as an argument that names a function directly) instead of storing it in a variable or selecting it at runtime
    }
    outBuf[0] = applyIt(gFn, 41);
}
