// Regression test for shader-slang/slang#12006.
//
// A `__extern_cpp` global with external linkage is exported with an unmangled name so host code can
// find it and set its value at runtime -- the documented `__global public __extern_cpp int
// myGlobal;` pattern (docs/cpu-target.md). It has no in-module initializer by design, so reading it
// must NOT warn E41017. This test is exhaustive and carries no annotations: were the false positive
// to reappear, the resulting E41017 would be an unannotated diagnostic and fail the test.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target cpp

[COM("007E2DAF-3A1F-4BC3-B08B-A9DB233D38CE")]
interface IThing
{
    int calc(int x);
}

export __global __extern_cpp IThing gThing;

export __extern_cpp int useThing(int x)
{
    return gThing.calc(x);
}
