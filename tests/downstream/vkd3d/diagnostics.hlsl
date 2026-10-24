//TEST:SIMPLE(filecheck=CHECK):-O1 -target dxbc -profile ps_5_0 -entry pixelMain -pass-through vkd3d
//CHECK: vkd3d
//CHECK: diagnostics.hlsl
//CHECK: E5005

float4 pixelMain() : SV_Target
{
    return missingValue;
}
