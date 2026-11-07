//TEST:SIMPLE(filecheck=CHECK):-target dxbc -profile ps_5_0 -entry pixelMain -pass-through vkd3d -O0
//TEST:SIMPLE(filecheck=CHECK):-O1 -target dxbc -profile ps_5_0 -entry pixelMain -pass-through vkd3d -g
//TEST:SIMPLE(filecheck=CHECK):-O1 -target dxbc -profile ps_5_0 -entry pixelMain -pass-through vkd3d -fp-mode precise
//TEST:SIMPLE(filecheck=CHECK):-O1 -target dxbc -profile ps_5_0 -entry pixelMain -pass-through vkd3d -Xvkd3d -unknown
//CHECK: vkd3d does not support

float4 pixelMain() : SV_Target
{
    return float4(1, 0, 0, 1);
}
