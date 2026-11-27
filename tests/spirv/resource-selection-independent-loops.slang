//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry main -stage fragment -O0 -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry main -stage fragment -O1 -validate-ir -validate-ir-detailed

// Check that two independent selection graphs sharing loop branches keep each other's
// argument slots, their array identities, the counter increment, and loop attributes.
// Each integer phi keeps its own initial index and receives only its own conditional update.
// The final array accesses use those phis directly instead of loads from local variables.
// CHECK: OpFunction
// CHECK: %[[ENTRY:[^ ]+]] = OpLabel
// CHECK: OpBranch %[[HEADER:[^ ]+]]{{( |$)}}
// CHECK: %[[HEADER]] = OpLabel
// CHECK: %[[A:[^ ]+]] = OpPhi %int %[[NEXT_A:[^ ]+]] %[[BACK_EDGE:[^ ]+]] %int_0 %[[ENTRY]]{{( |$)}}
// CHECK: %[[B:[^ ]+]] = OpPhi %int %[[NEXT_B:[^ ]+]] %[[BACK_EDGE]] %int_1 %[[ENTRY]]{{( |$)}}
// CHECK: %[[K:[^ ]+]] = OpPhi %uint %[[NEXT_K:[^ ]+]] %[[BACK_EDGE]] %uint_0 %[[ENTRY]]{{( |$)}}
// CHECK: OpLoopMerge %[[AFTER_LOOP:[^ ]+]] %[[BACK_EDGE]] DontUnroll
// CHECK: %[[FIRST_UV:[^ ]+]] = OpLoad %v2float %uv{{( |$)}}
// CHECK: %[[FIRST_X:[^ ]+]] = OpCompositeExtract %float %[[FIRST_UV]] 0{{( |$)}}
// CHECK: %[[FIRST_COND:[^ ]+]] = OpFOrdGreaterThan %bool %[[FIRST_X]] %float_0_5{{( |$)}}
// CHECK: %[[NEXT_A]] = OpSelect %int %[[FIRST_COND]] %int_1 %[[A]]{{( |$)}}
// CHECK: %[[SECOND_UV:[^ ]+]] = OpLoad %v2float %uv{{( |$)}}
// CHECK: %[[SECOND_Y:[^ ]+]] = OpCompositeExtract %float %[[SECOND_UV]] 1{{( |$)}}
// CHECK: %[[SECOND_COND:[^ ]+]] = OpFOrdGreaterThan %bool %[[SECOND_Y]] %float_0_5{{( |$)}}
// CHECK: %[[NEXT_B]] = OpSelect %int %[[SECOND_COND]] %int_0 %[[B]]{{( |$)}}
// CHECK: %[[NEXT_K]] = OpIAdd %uint %[[K]] %uint_1{{( |$)}}
// CHECK: %[[BACK_EDGE]] = OpLabel
// CHECK: OpBranch %[[HEADER]]{{( |$)}}
// CHECK: %[[AFTER_LOOP]] = OpLabel
// CHECK: %[[FIRST_POINTER:[^ ]+]] = OpAccessChain {{[^ ]+}} %firstSamplers %[[A]]{{( |$)}}
// CHECK: %[[FIRST_SAMPLER:[^ ]+]] = OpLoad {{[^ ]+}} %[[FIRST_POINTER]]{{( |$)}}
// CHECK: %[[FIRST_PAIR:[^ ]+]] = OpSampledImage {{[^ ]+}} {{[^ ]+}} %[[FIRST_SAMPLER]]{{( |$)}}
// CHECK: OpImageSampleExplicitLod {{[^ ]+}} %[[FIRST_PAIR]]{{( |$)}}
// CHECK: %[[SECOND_POINTER:[^ ]+]] = OpAccessChain {{[^ ]+}} %secondSamplers %[[B]]{{( |$)}}
// CHECK: %[[SECOND_SAMPLER:[^ ]+]] = OpLoad {{[^ ]+}} %[[SECOND_POINTER]]{{( |$)}}
// CHECK: %[[SECOND_PAIR:[^ ]+]] = OpSampledImage {{[^ ]+}} {{[^ ]+}} %[[SECOND_SAMPLER]]{{( |$)}}
// CHECK: OpImageSampleExplicitLod {{[^ ]+}} %[[SECOND_PAIR]]{{( |$)}}

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
