//TEST:SIMPLE(filecheck=SELECT): -target spirv-asm -stage fragment -entry nested -O0 -validate-ir
//TEST:SIMPLE(filecheck=SELECT): -target spirv-asm -stage fragment -entry nested -validate-ir
//TEST:SIMPLE(filecheck=LOOP): -target spirv-asm -stage fragment -entry swap -O0 -validate-ir
//TEST:SIMPLE(filecheck=LOOP): -target spirv-asm -stage fragment -entry swap -validate-ir

// Check that nested sampler selects become integer selects with correctly typed nonuniform
// indices, and that swapping samplers in a loop uses integer state instead of local sampler
// variables.

// SELECT: %a = OpSelect %int
// SELECT: %b = OpSelect %int
// SELECT: %selected = OpSelect %int {{.*}} %a %b
// SELECT: OpAccessChain {{.*}} %samplers %selected
// SELECT: OpImageSampleExplicitLod
// The two integer phis exchange their previous values on the same back edge,
// while the independent counter advances by one. Each final phi indexes its sampler.
// LOOP: OpFunction
// LOOP: %[[ENTRY:[^ ]+]] = OpLabel
// LOOP: OpBranch %[[HEADER:[^ ]+]]{{( |$)}}
// LOOP: %[[HEADER]] = OpLabel
// LOOP: %[[A:[^ ]+]] = OpPhi %int %[[B:[^ ]+]] %[[BACK_EDGE:[^ ]+]] %int_0 %[[ENTRY]]{{( |$)}}
// LOOP: %[[B]] = OpPhi %int %[[A]] %[[BACK_EDGE]] %int_1 %[[ENTRY]]{{( |$)}}
// LOOP: %[[K:[^ ]+]] = OpPhi %uint %[[NEXT_K:[^ ]+]] %[[BACK_EDGE]] %uint_0 %[[ENTRY]]{{( |$)}}
// LOOP: OpLoopMerge %[[AFTER_LOOP:[^ ]+]] %[[BACK_EDGE]] None
// LOOP: %[[NEXT_K]] = OpIAdd %uint %[[K]] %uint_1{{( |$)}}
// LOOP: %[[BACK_EDGE]] = OpLabel
// LOOP: OpBranch %[[HEADER]]{{( |$)}}
// LOOP: %[[AFTER_LOOP]] = OpLabel
// LOOP: %[[A_POINTER:[^ ]+]] = OpAccessChain {{[^ ]+}} %samplers %[[A]]{{( |$)}}
// LOOP: %[[A_SAMPLER:[^ ]+]] = OpLoad {{[^ ]+}} %[[A_POINTER]]{{( |$)}}
// LOOP: %[[A_PAIR:[^ ]+]] = OpSampledImage {{[^ ]+}} {{[^ ]+}} %[[A_SAMPLER]]{{( |$)}}
// LOOP: OpImageSampleExplicitLod {{[^ ]+}} %[[A_PAIR]]{{( |$)}}
// LOOP: %[[B_POINTER:[^ ]+]] = OpAccessChain {{[^ ]+}} %samplers %[[B]]{{( |$)}}
// LOOP: %[[B_SAMPLER:[^ ]+]] = OpLoad {{[^ ]+}} %[[B_POINTER]]{{( |$)}}
// LOOP: %[[B_PAIR:[^ ]+]] = OpSampledImage {{[^ ]+}} {{[^ ]+}} %[[B_SAMPLER]]{{( |$)}}
// LOOP: OpImageSampleExplicitLod {{[^ ]+}} %[[B_PAIR]]{{( |$)}}

SamplerState samplers[8];
Texture2D<float4> texture;
cbuffer Ubo { uint count; };

// Folding the outer select reads the nonuniform indices emitted for the inner selects.
// The NonUniformResourceIndex instructions wrapping them must have the index's integer
// type rather than TypeType.
float4 nested(float2 uv : TEXCOORD) : SV_TARGET
{
    SamplerState a = select(uv.x > 0.5, samplers[0], samplers[1]);
    SamplerState b = select(uv.y > 0.5, samplers[2], samplers[3]);
    SamplerState selected = select(uv.x > uv.y, a, b);
    return texture.SampleLevel(selected, uv, 0);
}

float4 swap(float2 uv : TEXCOORD) : SV_TARGET
{
    SamplerState a = samplers[0];
    SamplerState b = samplers[1];
    for (uint k = 0; k < count; k++)
    {
        SamplerState temporary = a;
        a = b;
        b = temporary;
    }
    return texture.SampleLevel(a, uv, 0) + texture.SampleLevel(b, uv, 0);
}
