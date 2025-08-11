//TEST:SIMPLE(filecheck=CHECK): -target spirv
//TEST:SIMPLE(filecheck=HLSL): -target hlsl -entry fragMain -profile ps_6_0

// CHECK: OpEntryPoint
// HLSL: float4 fragMain() : SV_TARGET

extern static const bool enableConditional = false;

struct Vertex
{
    Conditional<float3, enableConditional> color : COLOR;
}

[shader("fragment")]
float4 fragMain(Vertex v) : SV_Target
{
    if (let c = v.color.get())
    {
        // This block should not be executed.
        return float4(c, 1.0f);
    }
    return float4(0.0f, 0.0f, 0.0f, 1.0f);
}