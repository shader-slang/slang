//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry main -stage fragment -O0 -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry main -stage fragment -O1 -validate-ir -validate-ir-detailed

// Check that when two loop parameters receive the same back-edge resource, specializing
// one parameter's graph rewrites only that parameter's argument slot. The graph for a
// contains shared but not b, so b's slot keeps a resource until b is specialized.
// Detailed IR validation checks the intermediate resource/selector types. The final integer
// phis have distinct entry values and a shared back-edge value, and both feed their own sample.
// CHECK: %[[A:[^ ]+]] = OpPhi %int %[[SHARED:[^ ]+]] %[[BACK_EDGE:[^ ]+]] %int_0 %[[ENTRY:[^ ]+]]
// CHECK: %[[B:[^ ]+]] = OpPhi %int %[[SHARED]] %[[BACK_EDGE]] %int_1 %[[ENTRY]]
// CHECK: OpLoopMerge
// CHECK: %[[SHARED]] = OpSelect %int
// CHECK: OpAccessChain {{.*}} %samplers %[[A]]
// CHECK: OpImageSampleExplicitLod
// CHECK: OpAccessChain {{.*}} %samplers %[[B]]
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
