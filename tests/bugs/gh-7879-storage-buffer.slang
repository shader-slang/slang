//TEST:SIMPLE(filecheck=CHECK): -target spirv -profile spirv_1_3

//CHECK: OpCapability UniformAndStorageBuffer16BitAccess

struct st {
    half4 data;
    uint16_t count;
};
StructuredBuffer<st> inputBuffer;
RWStructuredBuffer<half2> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    half4 result = inputBuffer[0].data;
    uint16_t val = inputBuffer[0].count;
    outputBuffer[0] = result.xy;
}