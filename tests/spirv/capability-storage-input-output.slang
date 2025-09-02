//TEST:SIMPLE(filecheck=CHECK16): -target spirv -profile spirv_1_3 -DIN_HALF
//TEST:SIMPLE(filecheck=CHECK16): -target spirv -profile spirv_1_3 -DIN_UINT16
//TEST:SIMPLE(filecheck=CHECK16): -target spirv -profile spirv_1_3 -DOUT_HALF
//TEST:SIMPLE(filecheck=CHECK): -target spirv -profile spirv_1_3

//CHECK16: OpCapability StorageInputOutput16
//CHECK-NOT: OpCapability StorageInputOutput16

struct VertexInput {
#ifdef IN_HALF
    half4 position : POSITION;
#else
    float4 position : POSITION;
#endif
#ifdef IN_UINT16
    uint16_t id : ID;
#else
    uint32_t id : ID;
#endif
};

#ifdef OUT_HALF
#define OUT_TYPE half4
#else
#define OUT_TYPE float4
#endif

struct VertexOutput {
    float4 position : SV_POSITION;
    OUT_TYPE color : COLOR;
};

[shader("vertex")]
VertexOutput vertexMain(VertexInput input)
{
    VertexOutput output;
    output.position = float4(input.position);
    if (input.id == 0) {
        output.color = OUT_TYPE(input.position);
    } else {
        output.color = OUT_TYPE(0);
    }
    return output;
}
