//TEST:SIMPLE(filecheck=CHECK):-O1 -target dxbc-asm -profile vs_5_0 -entry vertexMain
//CHECK: vs_5_0
//CHECK: ret

[shader("vertex")]
float4 vertexMain(float3 position : POSITION) : SV_Position
{
    return float4(position, 1.0);
}
