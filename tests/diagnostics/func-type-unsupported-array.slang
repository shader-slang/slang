// Companion to func-type-unsupported-on-kernel-target.slang, covering #12367 for an array of
// function type.
//
// An array is emitted element-type-first, so it produces the same undefined spelling one level
// in -- `FixedArray<Slang_FuncType<int32_t, int32_t> , 2>` for C++, `array<, i32(2)>` for WGSL.
// A check that only looked at the type itself, rather than the element type it holds, would
// pass this through silently.
//
// `cuda` is absent for the same reason as in the write-only companion: it discards the dead
// array and emits no reference to the function type, so there is nothing to diagnose.

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

static functype(int) -> int gArr[2] = {addOne, addTwo};
//CHECK:                    ^^^^ function-typed value is not supported on this target
//CHECK:                    ^^^^ a function-typed value cannot be represented when generating code for the current target; it is supported only where the value can be resolved at compile time, so name the function directly at the call site (or pass it as an argument that names a function directly) instead of storing it in a variable or selecting it at runtime

RWStructuredBuffer<int> outBuf;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    gArr[0] = addTwo;
    outBuf[0] = 41;
}
