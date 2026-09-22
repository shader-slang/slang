//TEST:SIMPLE(filecheck=SCALAR): -target cuda -entry scalarUniform -stage compute
//TEST:SIMPLE(filecheck=AGGREGATE): -target cuda -entry aggregateUniform -stage compute

// CUDA stores the compiler's global parameter group in constant memory. Its scalar and aggregate
// fields must use ordinary loads, because __ldg only accepts addresses in global memory.
// constant-buffer-ldg.slang separately verifies that a ConstantBuffer's pointed-to device data
// still uses __ldg after its pointer is loaded from the constant-memory parameter group.

struct Settings
{
    uint count;
    float3 scale;
};

uniform uint frame;
uniform Settings settings;
RWStructuredBuffer<uint> outputBuffer;
RWStructuredBuffer<Settings> aggregateOutput;

// Bracket the entire output so generated aggregate-load helpers cannot hide an invalid __ldg.
//SCALAR-NOT: __ldg
//SCALAR: __constant__
//SCALAR-NOT: __ldg

[shader("compute")]
[numthreads(1, 1, 1)]
void scalarUniform()
{
    outputBuffer[0] = frame;
}

//AGGREGATE-NOT: __ldg
//AGGREGATE: __constant__
//AGGREGATE-NOT: __ldg

[shader("compute")]
[numthreads(1, 1, 1)]
void aggregateUniform()
{
    aggregateOutput[0] = settings;
}
