//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry main -stage fragment -O0 -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry main -stage fragment -O1 -validate-ir -validate-ir-detailed

// This test checks that mutually dependent loop parameters and shared sampler selects become
// integer state. The three samplers and the loop counter share branch argument lists,
// so replacing the resource parameters must also preserve the counter's position.
// The shared select feeds two later selects, requiring its index to be available to
// both even when the graph walk reaches those users in a different order.
// CHECK: OpStore %a %int_0
// CHECK: OpStore %b %int_1
// CHECK: OpStore %c %int_2
// CHECK: OpStore %k %uint_0
// CHECK: OpLoopMerge
// CHECK: %[[COND_X:[^ ]+]] = OpFOrdGreaterThan %bool
// CHECK: %[[A:[^ ]+]] = OpLoad %int %a
// CHECK: %[[B:[^ ]+]] = OpLoad %int %b
// CHECK: %[[SHARED:[^ ]+]] = OpSelect %int %[[COND_X]] %[[A]] %[[B]]
// CHECK: %[[COND_Y:[^ ]+]] = OpFOrdGreaterThan %bool
// CHECK: %[[C:[^ ]+]] = OpLoad %int %c
// CHECK: %[[NEXT:[^ ]+]] = OpSelect %int %[[COND_Y]] %[[SHARED]] %[[C]]
// CHECK: %[[SELECTED:[^ ]+]] = OpSelect %int {{[^ ]+}} %[[SHARED]] %[[NEXT]]
// CHECK: %[[K:[^ ]+]] = OpLoad %uint %k
// CHECK: %[[NEXT_K:[^ ]+]] = OpIAdd %uint %[[K]] %uint_1
// CHECK: OpStore %a %[[SELECTED]]
// CHECK: OpStore %b %[[NEXT]]
// CHECK: OpStore %c %[[SHARED]]
// CHECK: OpStore %k %[[NEXT_K]]
// CHECK: OpImageSampleExplicitLod
// CHECK: OpImageSampleExplicitLod
// CHECK: OpImageSampleExplicitLod

SamplerState samplers[3];
Texture2D<float4> texture;
cbuffer Params { uint count; };

float4 main(float2 uv : TEXCOORD) : SV_Target
{
    SamplerState a = samplers[0];
    SamplerState b = samplers[1];
    SamplerState c = samplers[2];
    for (uint k = 0; k < count; ++k)
    {
        SamplerState shared = select(uv.x > 0.5, a, b);
        SamplerState next = select(uv.y > 0.5, shared, c);
        a = select(uv.x > uv.y, shared, next);
        b = next;
        c = shared;
    }
    return texture.SampleLevel(a, uv, 0)
        + 2 * texture.SampleLevel(b, uv, 0)
        + 4 * texture.SampleLevel(c, uv, 0);
}
