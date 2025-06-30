//TEST:SIMPLE(filecheck=SPIRV): -target spirv -entry main -stage fragment -allow-glsl
//TEST:SIMPLE(filecheck=GLSL): -target glsl -entry main -stage fragment -allow-glsl

// SPIRV: OpDecorate %{{.*}} BuiltIn DeviceIndex
// GLSL: gl_DeviceIndex

struct PSIn
{
    float3 color;
}

[shader("fragment")]
float4 main(PSIn pin)
{
    return float4(pin.color, (float)gl_DeviceIndex);
}
