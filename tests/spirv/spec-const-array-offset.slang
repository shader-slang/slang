//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm

// CHECK-DAG: OpMemberDecorate %Resources_std140 0 Offset 0
// CHECK-DAG: OpMemberDecorate %Resources_std140 1 Offset 4
// CHECK-DAG: OpMemberDecorate %Resources_std140 2 Offset 16

[SpecializationConstant] const uint MAX_SAMPLES = 32;

struct Resources {
    float first;
    float second;
    float4 samples[MAX_SAMPLES];
}

ParameterBlock<Resources> resources;

RWStructuredBuffer<float4> outputBuffer;

[shader("compute")]
[numthreads(16, 1, 1)]
void main(uint3 id : SV_DispatchThreadID) {
    outputBuffer[id.x] = resources.samples[id.x] + resources.first + resources.second;
}
