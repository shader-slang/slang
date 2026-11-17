//TEST:SIMPLE(filecheck=SPV): -target spirv -g0 -entry computeMain -stage compute
// `-g0` is load-bearing here in the opposite direction from the regression
// test: with debug info, materialization keeps the first store alive, so the
// `SPV-NOT: %float_11` check would fail even with the fix in.

// Test that the resource-content read check does not over-reach: a store into
// a function-local aggregate var is optimized away across a buffer load,
// because var memory never overlaps the contents of a resource, so the load
// must not block the elimination. The dynamic index keeps the array out of
// scalar replacement so the store stays a real var access.

struct Big { float x[16]; };

[noinline]
float localVarStoreAcrossBufferLoad(int idx)
{
    Big local;
    local.x[idx] = 11.0f;
    float s = bufB.Load(0);
    local.x[idx] = 22.0f;
    return s + local.x[idx];
}

RWStructuredBuffer<float> bufB;
RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = localVarStoreAcrossBufferLoad(0);
}

// SPV: OpEntryPoint
// The first store's constant is eliminated with the store; only 22 survives.
// SPV-NOT: %float_11 = OpConstant
// SPV: %float_22 = OpConstant
