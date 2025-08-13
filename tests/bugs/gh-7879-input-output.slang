//TEST:SIMPLE(filecheck=CHECK): -target spirv -profile spirv_1_3

//CHECK: OpCapability StorageInputOutput16

struct VertexInput {
    half4 position : POSITION;
    uint16_t id : ID;
};

struct VertexOutput {
    float4 position : SV_POSITION;
    half4 color : COLOR;
};

[shader("vertex")]
VertexOutput vertexMain(VertexInput input)
{
    VertexOutput output;
    output.position = float4(input.position);
    output.color = input.position; // half4 output
    return output;
}