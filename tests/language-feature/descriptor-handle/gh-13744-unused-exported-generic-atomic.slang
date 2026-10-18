// An unused `export`ed generic is removed before SPIR-V legalization (see
// gh-13744-exported-override-combined-sampler.slang), together with every function that only it
// calls. Their atomic operations are still checked first, with the same rule as the other targets:
// a function-local destination is invalid, while an `inout` parameter destination cannot be
// validated without a call site and is accepted. See shader-slang/slang#13744.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -stage compute -entry computeMain
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -stage compute -entry computeMain

RWStructuredBuffer<int> outputBuffer;

export int bumpLocal<T>(T v)
{
    int x = 0;
    InterlockedAdd(x, 1);
//CHECK:          ^ invalid atomic destination
//CHECK:          ^ cannot perform atomic operation because destination is neither groupshared nor from a device buffer.
    return x;
}

export void bumpParam<T>(inout int x, T v)
{
    InterlockedAdd(x, 1);
}

int genericHelper<T>(T v)
{
    int x = 0;
    InterlockedAdd(x, 1);
//CHECK:          ^ invalid atomic destination
//CHECK:          ^ cannot perform atomic operation because destination is neither groupshared nor from a device buffer.
    return x;
}

int plainHelper()
{
    int x = 0;
    InterlockedAdd(x, 1);
//CHECK:          ^ invalid atomic destination
//CHECK:          ^ cannot perform atomic operation because destination is neither groupshared nor from a device buffer.
    return x;
}

export int callHelpers<T>(T v)
{
    return genericHelper(v) + plainHelper();
}

// A valid destination stays accepted even though the variable is removed with the generic.
groupshared int sharedCounter;

export int bumpShared<T>(T v)
{
    InterlockedAdd(sharedCounter, 1);
    return 0;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = 1;
}
