//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry main -stage fragment -O0 -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry main -stage fragment -O1 -validate-ir -validate-ir-detailed

// Check that when two loop parameters receive the same back-edge resource, specializing
// one parameter's graph rewrites only that parameter's argument slot. The graph for a
// contains shared but not b, so b's slot keeps a resource until b is specialized.
// The CHECK lines are deliberately light: detailed IR validation fails if b's slot is
// given an integer while b still has a resource type.
// CHECK-DAG: OpStore %a %int_0
// CHECK-DAG: OpStore %b %int_1
// CHECK: OpLoopMerge
// CHECK: OpSelect %int
// CHECK: OpImageSampleExplicitLod
// CHECK: OpImageSampleExplicitLod

SamplerState samplers[2];
Texture2D<float4> texture;
cbuffer Params { uint count; };

float4 main(float2 uv : TEXCOORD) : SV_Target
{
    SamplerState a = samplers[0];
    SamplerState b = samplers[1];
    [loop]
    for (uint k = 0; k < count; ++k)
    {
        SamplerState shared = select(uv.x > float(k), samplers[0], samplers[1]);
        a = shared;
        b = shared;
    }
    return texture.SampleLevel(a, uv, 0) + 2 * texture.SampleLevel(b, uv, 0);
}
