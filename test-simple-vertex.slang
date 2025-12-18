struct VertexOutput
{
    float4 position : SV_Position;
    float3 color : COLOR0;
};

[shader("vertex")]
VertexOutput vertexMain() 
{
    VertexOutput output;
    output.position = float4(0, 0, 0, 1);
    output.color = float3(1, 0, 0);
    return output;
}
