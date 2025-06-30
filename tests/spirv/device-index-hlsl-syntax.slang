//TEST:SIMPLE(filecheck=SPIRV): -target spirv -entry main
//TEST:SIMPLE(filecheck=GLSL): -target glsl -entry main

// SPIRV: OpDecorate %{{.*}} BuiltIn DeviceIndex
// GLSL: gl_DeviceIndex

struct PSIn
{
    float3 color;
    int deviceIndex : SV_DeviceIndex;
}

[shader("fragment")]
float4 main(PSIn pin)
{
    return float4(pin.color, (float)pin.deviceIndex);
}
