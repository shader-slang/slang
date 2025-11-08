//TEST:SIMPLE(filecheck=CHECK): -target spirv

// CHECK-DAG: OpSpecConstant %uint 32
// CHECK-DAG: OpTypeArray %v4float

[SpecializationConstant] const uint MAX_SAMPLES = 32;

struct Resources {
    float4 samples[MAX_SAMPLES];
}

ParameterBlock<Resources> resources;

//TEST_INPUT:ubuffer(data=[0], stride=16):out,name=outputBuffer
RWStructuredBuffer<float4> outputBuffer;

[shader("compute")]
[numthreads(16, 16, 1)]
void main(uint3 id : SV_DispatchThreadID) {
    if (id.x < MAX_SAMPLES)
    {
        outputBuffer[id.x] = resources.samples[id.x];
    }
}
