//TEST:SIMPLE(filecheck=CHECKA): -target spirv -profile spirv_1_3 -DIN_HALF
//TEST:SIMPLE(filecheck=CHECKA): -target spirv -profile spirv_1_3 -DIN_UINT16
//TEST:SIMPLE(filecheck=CHECKA): -target spirv -profile spirv_1_3 -DOUT_HALF
//TEST:SIMPLE(filecheck=CHECKA): -target spirv -profile spirv_1_3 -DOUT_UINT16
//TEST:SIMPLE(filecheck=CHECKB): -target spirv -profile spirv_1_3

//CHECKA: OpCapability UniformAndStorageBuffer16BitAccess
//CHECKB-NOT: OpCapability UniformAndStorageBuffer16BitAccess


uniform struct {
#ifdef IN_HALF
    half4 data;
#else
    float4 data;
#endif
#ifdef IN_UINT16
    uint16_t index;
#else
    uint32_t index;
#endif
} inputBuffer;

#ifdef OUT_HALF
#define OUT_FLOAT_TYPE half4
#else
#define OUT_FLOAT_TYPE float4
#endif
#ifdef OUT_UINT16
#define OUT_UINT_TYPE uint16_t
#else
#define OUT_UINT_TYPE uint32_t
#endif

struct st {
    OUT_FLOAT_TYPE data;
    OUT_UINT_TYPE index;
};
RWStructuredBuffer<st> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0].data = OUT_FLOAT_TYPE(inputBuffer.data);
    outputBuffer[1].index = OUT_UINT_TYPE(inputBuffer.index);
}
