//TEST:SIMPLE(filecheck=CHECK8): -target spirv -profile spirv_1_3 -DIN_UINT8
//TEST:SIMPLE(filecheck=CHECK16): -target spirv -profile spirv_1_3 -DIN_UINT16
//TEST:SIMPLE(filecheck=CHECK16): -target spirv -profile spirv_1_3 -DIN_HALF
//TEST:SIMPLE(filecheck=CHECKBOTH): -target spirv -profile spirv_1_3 -DIN_UINT8 -DIN_HALF
//TEST:SIMPLE(filecheck=CHECK8): -target spirv -profile spirv_1_3 -DOUT_UINT8
//TEST:SIMPLE(filecheck=CHECK16): -target spirv -profile spirv_1_3 -DOUT_UINT16
//TEST:SIMPLE(filecheck=CHECK16): -target spirv -profile spirv_1_3 -DOUT_HALF
//TEST:SIMPLE(filecheck=CHECK16): -target spirv -profile spirv_1_3 -DOUT_HALF -DATOMIC
//TEST:SIMPLE(filecheck=CHECK): -target spirv -profile spirv_1_3

//CHECK8: OpCapability UniformAndStorageBuffer8BitAccess
//CHECK8-NOT: OpCapability UniformAndStorageBuffer16BitAccess
//CHECK16: OpCapability UniformAndStorageBuffer16BitAccess
//CHECK16-NOT: OpCapability UniformAndStorageBuffer8BitAccess
//CHECKBOTH-DAG: OpCapability UniformAndStorageBuffer8BitAccess
//CHECKBOTH-DAG: OpCapability UniformAndStorageBuffer16BitAccess
//CHECK-NOT: OpCapability UniformAndStorageBuffer8BitAccess
//CHECK-NOT: OpCapability UniformAndStorageBuffer16BitAccess


uniform struct {
#if defined(IN_HALF)
    half4 data;
#else
    float4 data;
#endif
#if defined(IN_UINT8)
    uint8_t index;
#elif defined(IN_UINT16)
    uint16_t index;
#else
    uint32_t index;
#endif
} inputBuffer;

#if defined(OUT_HALF)
#define OUT_FLOAT_TYPE half
#else
#define OUT_FLOAT_TYPE float
#endif
#if defined(OUT_UINT8)
#define OUT_UINT_TYPE uint8_t
#elif defined(OUT_UINT16)
#define OUT_UINT_TYPE uint16_t
#else
#define OUT_UINT_TYPE uint32_t
#endif

struct st {
#if defined(ATOMIC)
    Atomic<OUT_FLOAT_TYPE> data;
#else
    OUT_FLOAT_TYPE data;
#endif
    OUT_UINT_TYPE index;
};
RWStructuredBuffer<st> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0].data = OUT_FLOAT_TYPE(inputBuffer.data.x);
    outputBuffer[1].index = OUT_UINT_TYPE(inputBuffer.index);
}
