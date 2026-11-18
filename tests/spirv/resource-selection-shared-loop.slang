//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry main -stage fragment -O0 -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry main -stage fragment -O1 -validate-ir -validate-ir-detailed

// Check that mutually dependent loop parameters and shared sampler selects become
// integer state. The three samplers and the loop counter share branch argument lists,
// so replacing the resource parameters must also preserve the counter's position.
// The shared select feeds two later selects, requiring its index to be available to
// both even when the graph walk reaches those users in a different order.
// CHECK: OpFunction
// CHECK: %[[ENTRY:[^ ]+]] = OpLabel
// CHECK: OpBranch %[[HEADER:[^ ]+]]{{( |$)}}
// CHECK: %[[HEADER]] = OpLabel
// CHECK: %[[A:[^ ]+]] = OpPhi %int %[[SELECTED:[^ ]+]] %[[BACK_EDGE:[^ ]+]] %int_0 %[[ENTRY]]{{( |$)}}
// CHECK: %[[B:[^ ]+]] = OpPhi %int %[[NEXT:[^ ]+]] %[[BACK_EDGE]] %int_1 %[[ENTRY]]{{( |$)}}
// CHECK: %[[C:[^ ]+]] = OpPhi %int %[[SHARED:[^ ]+]] %[[BACK_EDGE]] %int_2 %[[ENTRY]]{{( |$)}}
// CHECK: %[[K:[^ ]+]] = OpPhi %uint %[[NEXT_K:[^ ]+]] %[[BACK_EDGE]] %uint_0 %[[ENTRY]]{{( |$)}}
// CHECK: OpLoopMerge %[[AFTER_LOOP:[^ ]+]] %[[BACK_EDGE]] None
// CHECK: %[[COND_X:[^ ]+]] = OpFOrdGreaterThan %bool
// CHECK: %[[SHARED]] = OpSelect %int %[[COND_X]] %[[A]] %[[B]]{{( |$)}}
// CHECK: %[[COND_Y:[^ ]+]] = OpFOrdGreaterThan %bool
// CHECK: %[[NEXT]] = OpSelect %int %[[COND_Y]] %[[SHARED]] %[[C]]{{( |$)}}
// CHECK: %[[SELECTED]] = OpSelect %int {{[^ ]+}} %[[SHARED]] %[[NEXT]]{{( |$)}}
// CHECK: %[[NEXT_K]] = OpIAdd %uint %[[K]] %uint_1{{( |$)}}
// CHECK: %[[BACK_EDGE]] = OpLabel
// CHECK: OpBranch %[[HEADER]]{{( |$)}}
// CHECK: %[[AFTER_LOOP]] = OpLabel
// CHECK: %[[A_POINTER:[^ ]+]] = OpAccessChain {{[^ ]+}} %samplers %[[A]]{{( |$)}}
// CHECK: %[[A_SAMPLER:[^ ]+]] = OpLoad {{[^ ]+}} %[[A_POINTER]]{{( |$)}}
// CHECK: %[[A_PAIR:[^ ]+]] = OpSampledImage {{[^ ]+}} {{[^ ]+}} %[[A_SAMPLER]]{{( |$)}}
// CHECK: OpImageSampleExplicitLod {{[^ ]+}} %[[A_PAIR]]{{( |$)}}
// CHECK: %[[B_POINTER:[^ ]+]] = OpAccessChain {{[^ ]+}} %samplers %[[B]]{{( |$)}}
// CHECK: %[[B_SAMPLER:[^ ]+]] = OpLoad {{[^ ]+}} %[[B_POINTER]]{{( |$)}}
// CHECK: %[[B_PAIR:[^ ]+]] = OpSampledImage {{[^ ]+}} {{[^ ]+}} %[[B_SAMPLER]]{{( |$)}}
// CHECK: OpImageSampleExplicitLod {{[^ ]+}} %[[B_PAIR]]{{( |$)}}
// CHECK: %[[C_POINTER:[^ ]+]] = OpAccessChain {{[^ ]+}} %samplers %[[C]]{{( |$)}}
// CHECK: %[[C_SAMPLER:[^ ]+]] = OpLoad {{[^ ]+}} %[[C_POINTER]]{{( |$)}}
// CHECK: %[[C_PAIR:[^ ]+]] = OpSampledImage {{[^ ]+}} {{[^ ]+}} %[[C_SAMPLER]]{{( |$)}}
// CHECK: OpImageSampleExplicitLod {{[^ ]+}} %[[C_PAIR]]{{( |$)}}

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
