//TEST:SIMPLE(filecheck=ROW):-O1 -target dxbc-asm -profile vs_5_0 -entry vertexMain -pass-through vkd3d -matrix-layout-row-major
//TEST:SIMPLE(filecheck=COLUMN):-O1 -target dxbc-asm -profile vs_5_0 -entry vertexMain -pass-through vkd3d -matrix-layout-column-major
//ROW: vs_5_0
//ROW: dcl_constantBuffer cb0[2]
//COLUMN: vs_5_0
//COLUMN: dcl_constantBuffer cb0[3]

cbuffer Constants : register(b0)
{
    float2x3 transform;
};

float4 vertexMain(float3 position : POSITION) : SV_Position
{
    return float4(mul(transform, position), 0, 1);
}
