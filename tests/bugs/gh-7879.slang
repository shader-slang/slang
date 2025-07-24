//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -profile spirv_1_3

//CHECK: OpCapability UniformAndStorageBuffer16BitAccess

struct st {
    half4 test;
};
RWStructuredBuffer<st> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0].test = half4(float4(1.0, 2.0, 3.0, 4.0));
}