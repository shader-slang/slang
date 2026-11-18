//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -stage fragment -entry main -capability spvDescriptorHeapEXT -O0 -validate-ir
//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -stage fragment -entry main -capability spvDescriptorHeapEXT -validate-ir -O1

// The sampler's heap index starts at zero. Even iterations replace it with `i + k`, while odd
// iterations keep its previous value. The loop-header phi receives the conditional merge's
// result, whose incoming values distinguish the update and skip paths. Following the header
// value into the sampled sampler also rules out an unconditional replacement after the merge.
// We separately follow the texture's `i`/`j` heap index.

// CHECK: OpFunction
// CHECK: %[[ENTRY:[^ ]+]] = OpLabel
// CHECK: OpBranch %[[HEADER:[^ ]+]]{{( |$)}}
// CHECK: %[[HEADER]] = OpLabel
// CHECK: %[[S:[^ ]+]] = OpPhi %uint %[[NEXT_S:[^ ]+]] %[[BACK_EDGE:[^ ]+]] %uint_0 %[[ENTRY]]{{( |$)}}
// CHECK: %[[K:[^ ]+]] = OpPhi %uint %[[NEW_K:[^ ]+]] %[[BACK_EDGE]] %uint_0 %[[ENTRY]]{{( |$)}}
// CHECK: OpLoopMerge %[[AFTER_LOOP:[^ ]+]] %[[BACK_EDGE]] None
// CHECK: %[[X_POINTER:[^ ]+]] = OpAccessChain {{[^ ]+}} %Ubo %int_0{{( |$)}}
// CHECK: %[[COUNT:[^ ]+]] = OpLoad %uint %[[X_POINTER]]{{( |$)}}
// CHECK: %[[CONTINUE:[^ ]+]] = OpULessThan %bool %[[K]] %[[COUNT]]{{( |$)}}
// CHECK: OpBranchConditional %[[CONTINUE]] %[[BODY:[^ ]+]] {{%[^ ]+}}{{( |$)}}
// CHECK: %[[BODY]] = OpLabel
// CHECK: %[[PARITY:[^ ]+]] = OpUMod %uint %[[K]] %uint_2{{( |$)}}
// CHECK: %[[IS_EVEN:[^ ]+]] = OpIEqual %bool %[[PARITY]] %uint_0{{( |$)}}
// CHECK: OpSelectionMerge %[[NEXT_ITERATION:[^ ]+]] None
// CHECK: OpBranchConditional %[[IS_EVEN]] %[[UPDATE:[^ ]+]] %[[SKIP:[^ ]+]]{{( |$)}}
// CHECK: %[[UPDATE]] = OpLabel
// CHECK: %[[I_POINTER:[^ ]+]] = OpAccessChain {{[^ ]+}} %Ubo %int_1{{( |$)}}
// CHECK: %[[I:[^ ]+]] = OpLoad %uint %[[I_POINTER]]{{( |$)}}
// CHECK: %[[NEXT:[^ ]+]] = OpIAdd %uint %[[I]] %[[K]]{{( |$)}}
// CHECK-NOT: OpLabel
// CHECK: OpBranch %[[NEXT_ITERATION]]{{( |$)}}
// CHECK: %[[NEXT_ITERATION]] = OpLabel
// CHECK: %[[NEXT_S]] = OpPhi %uint %[[NEXT]] %[[UPDATE]] %[[S]] %[[SKIP]]{{( |$)}}
// CHECK: %[[NEW_K]] = OpIAdd %uint %[[K]] %uint_1{{( |$)}}
// CHECK: OpBranch %[[BACK_EDGE]]{{( |$)}}
// CHECK: %[[BACK_EDGE]] = OpLabel
// CHECK: OpBranch %[[HEADER]]{{( |$)}}
// CHECK: %[[AFTER_LOOP]] = OpLabel
// CHECK: %[[X:[^ ]+]] = OpLoad %uint %[[X_POINTER]]{{( |$)}}
// CHECK: %[[CHOOSE_I:[^ ]+]] = OpIEqual %bool %[[X]] %uint_1{{( |$)}}
// CHECK: %[[TEXTURE_I_POINTER:[^ ]+]] = OpAccessChain {{[^ ]+}} %Ubo %int_1{{( |$)}}
// CHECK: %[[TEXTURE_I:[^ ]+]] = OpLoad %uint %[[TEXTURE_I_POINTER]]{{( |$)}}
// CHECK: %[[J_POINTER:[^ ]+]] = OpAccessChain {{[^ ]+}} %Ubo %int_2{{( |$)}}
// CHECK: %[[J:[^ ]+]] = OpLoad %uint %[[J_POINTER]]{{( |$)}}
// CHECK: %[[TEXTURE:[^ ]+]] = OpSelect %uint %[[CHOOSE_I]] %[[TEXTURE_I]] %[[J]]{{( |$)}}
// CHECK: %[[TEXTURE_POINTER:[^ ]+]] = OpUntypedAccessChainKHR {{.*}} %slang_resourceHeap %[[TEXTURE]]{{( |$)}}
// CHECK: %[[IMAGE:[^ ]+]] = OpLoad {{[^ ]+}} %[[TEXTURE_POINTER]]{{( |$)}}
// CHECK: %[[SAMPLER_POINTER:[^ ]+]] = OpUntypedAccessChainKHR {{.*}} %slang_samplerHeap %[[S]]{{( |$)}}
// CHECK: %[[SAMPLER:[^ ]+]] = OpLoad {{[^ ]+}} %[[SAMPLER_POINTER]]{{( |$)}}
// CHECK: %[[PAIR:[^ ]+]] = OpSampledImage {{[^ ]+}} %[[IMAGE]] %[[SAMPLER]]{{( |$)}}
// CHECK: OpImageSampleImplicitLod {{[^ ]+}} %[[PAIR]]{{( |$)}}

cbuffer Ubo { uint x; uint i; uint j; };

float4 main(float2 uv : TEXCOORD) : SV_TARGET
{
    SamplerState s = SamplerDescriptorHeap[0];
    for (uint k = 0; k < x; k++)
    {
        if (k % 2 == 0)
            s = SamplerDescriptorHeap[i + k];
    }
    Texture2D<float4> t = select(x == 1,
        (Texture2D<float4>)ResourceDescriptorHeap[i],
        (Texture2D<float4>)ResourceDescriptorHeap[j]);
    return t.Sample(s, uv);
}
