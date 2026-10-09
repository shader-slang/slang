//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry main -stage fragment -O0 -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry main -stage fragment -O1 -validate-ir -validate-ir-detailed

// Check that two independent selection graphs sharing loop branches keep each other's
// argument slots, their array identities, the counter increment, and loop attributes.
// Each OpAccessChain names the load that supplies its index, so other loads of a or b
// do not affect matching.
// CHECK-DAG: OpLoopMerge {{.*}} DontUnroll
// CHECK-DAG: OpAccessChain {{[^ ]+}} %firstSamplers %[[FIRST_INDEX:[^ ]+]]
// CHECK-DAG: %[[FIRST_INDEX]] = OpLoad %int %a
// CHECK-DAG: OpAccessChain {{[^ ]+}} %secondSamplers %[[SECOND_INDEX:[^ ]+]]
// CHECK-DAG: %[[SECOND_INDEX]] = OpLoad %int %b
// CHECK-DAG: OpSelect %int
// CHECK-DAG: OpSelect %int
// CHECK-DAG: %[[NEXT_K:[^ ]+]] = OpIAdd %uint {{[^ ]+}} %uint_1
// CHECK-DAG: OpStore %k %[[NEXT_K]]
// CHECK-DAG: OpImageSampleExplicitLod
// CHECK-DAG: OpImageSampleExplicitLod

SamplerState firstSamplers[2];
SamplerState secondSamplers[2];
Texture2D<float4> texture;
cbuffer Params { uint count; };

float4 main(float2 uv : TEXCOORD) : SV_Target
{
    SamplerState a = firstSamplers[0];
    SamplerState b = secondSamplers[1];
    [loop]
    for (uint k = 0; k < count; ++k)
    {
        a = select(uv.x > 0.5, firstSamplers[1], a);
        b = select(uv.y > 0.5, secondSamplers[0], b);
    }
    return texture.SampleLevel(a, uv, 0) + 2 * texture.SampleLevel(b, uv, 0);
}
