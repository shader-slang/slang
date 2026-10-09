//TEST:SIMPLE(filecheck=CHECK): -target glsl -entry main -stage fragment -O0 -validate-ir
//TEST:SIMPLE(filecheck=CHECK): -target glsl -entry main -stage fragment -O1 -validate-ir

// Check that swapping two samplers and then breaking out of a loop preserves their
// identities. Resource specialization replaces the samplers with nonuniform array indices,
// so GLSL must save both old indices before assigning either one. The different sample
// weights make a lost or duplicated sampler observable in the result.
// CHECK-DAG: int [[OLD_B:[A-Za-z_0-9]+]] = b_0;
// CHECK-DAG: int [[OLD_A:[A-Za-z_0-9]+]] = a_0;
// CHECK: a_0 = nonuniformEXT([[OLD_B]]);
// CHECK: b_0 = nonuniformEXT([[OLD_A]]);

SamplerState samplers[2];
Texture2D<float4> texture;
cbuffer Params
{
    uint count;
};
float4 main(float2 uv : TEXCOORD) : SV_Target
{
    SamplerState a = samplers[0], b = samplers[1];
    for (uint k = 0; k < count; ++k)
    {
        SamplerState oldA = a;
        a = b;
        b = oldA;
        if (uv.y > 0.5)
            break;
    }
    return texture.SampleLevel(a, uv, 0) + 2 * texture.SampleLevel(b, uv, 0);
}
