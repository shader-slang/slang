//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -stage fragment -entry main -capability spvDescriptorHeapEXT -O0 -validate-ir
//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -stage fragment -entry main -capability spvDescriptorHeapEXT -validate-ir -O1
//TEST:SIMPLE(filecheck=STORES): -target spirv-asm -stage fragment -entry main -capability spvDescriptorHeapEXT -O0 -validate-ir
//TEST:SIMPLE(filecheck=STORES): -target spirv-asm -stage fragment -entry main -capability spvDescriptorHeapEXT -O1 -validate-ir

// The sampler's heap index starts at zero. Even iterations replace it with `i + k`, while odd
// iterations keep its previous value. We follow the parity test through the update and skip
// paths, then follow the final index into the sampled sampler. The separate `STORES` checks
// allow only the initialization and conditional update, so an extra unconditional write cannot
// hide behind the correct update arm. We also check the texture's `i`/`j` heap index.

// CHECK: %[[S:[^ ]+]] = OpVariable %_ptr_Function_uint Function
// CHECK: %[[K:[^ ]+]] = OpVariable %_ptr_Function_uint Function
// CHECK: OpStore %[[S]] %uint_0{{( |$)}}
// CHECK: OpStore %[[K]] %uint_0{{( |$)}}
// CHECK: OpBranch %[[HEADER:[^ ]+]]{{( |$)}}
// CHECK: %[[HEADER]] = OpLabel
// CHECK: OpLoopMerge %[[AFTER_LOOP:[^ ]+]] %[[BACK_EDGE:[^ ]+]] None
// CHECK: %[[X_POINTER:[^ ]+]] = OpAccessChain {{[^ ]+}} %Ubo %int_0{{( |$)}}
// CHECK: %[[COUNT:[^ ]+]] = OpLoad %uint %[[X_POINTER]]{{( |$)}}
// CHECK: %[[CURRENT_K:[^ ]+]] = OpLoad %uint %[[K]]{{( |$)}}
// CHECK: %[[CONTINUE:[^ ]+]] = OpULessThan %bool %[[CURRENT_K]] %[[COUNT]]{{( |$)}}
// CHECK: OpBranchConditional %[[CONTINUE]] %[[BODY:[^ ]+]] {{%[^ ]+}}{{( |$)}}
// CHECK: %[[BODY]] = OpLabel
// CHECK: %[[PARITY_K:[^ ]+]] = OpLoad %uint %[[K]]{{( |$)}}
// CHECK: %[[PARITY:[^ ]+]] = OpUMod %uint %[[PARITY_K]] %uint_2{{( |$)}}
// CHECK: %[[IS_EVEN:[^ ]+]] = OpIEqual %bool %[[PARITY]] %uint_0{{( |$)}}
// CHECK: OpSelectionMerge %[[NEXT_ITERATION:[^ ]+]] None
// CHECK: OpBranchConditional %[[IS_EVEN]] %[[UPDATE:[^ ]+]] %[[NEXT_ITERATION]]{{( |$)}}
// CHECK: %[[UPDATE]] = OpLabel
// CHECK: %[[I_POINTER:[^ ]+]] = OpAccessChain {{[^ ]+}} %Ubo %int_1{{( |$)}}
// CHECK: %[[I:[^ ]+]] = OpLoad %uint %[[I_POINTER]]{{( |$)}}
// CHECK: %[[UPDATE_K:[^ ]+]] = OpLoad %uint %[[K]]{{( |$)}}
// CHECK: %[[NEXT:[^ ]+]] = OpIAdd %uint %[[I]] %[[UPDATE_K]]{{( |$)}}
// CHECK: OpStore %[[S]] %[[NEXT]]{{( |$)}}
// CHECK-NOT: OpLabel
// CHECK: OpBranch %[[NEXT_ITERATION]]{{( |$)}}
// CHECK: %[[NEXT_ITERATION]] = OpLabel
// CHECK: %[[OLD_K:[^ ]+]] = OpLoad %uint %[[K]]{{( |$)}}
// CHECK: %[[NEW_K:[^ ]+]] = OpIAdd %uint %[[OLD_K]] %uint_1{{( |$)}}
// CHECK: OpStore %[[K]] %[[NEW_K]]{{( |$)}}
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
// CHECK: %[[INDEX:[^ ]+]] = OpLoad %uint %[[S]]{{( |$)}}
// CHECK: %[[SAMPLER_POINTER:[^ ]+]] = OpUntypedAccessChainKHR {{.*}} %slang_samplerHeap %[[INDEX]]{{( |$)}}
// CHECK: %[[SAMPLER:[^ ]+]] = OpLoad {{[^ ]+}} %[[SAMPLER_POINTER]]{{( |$)}}
// CHECK: %[[PAIR:[^ ]+]] = OpSampledImage {{[^ ]+}} %[[IMAGE]] %[[SAMPLER]]{{( |$)}}
// CHECK: OpImageSampleImplicitLod {{[^ ]+}} %[[PAIR]]{{( |$)}}
// STORES: %[[S:[^ ]+]] = OpVariable %_ptr_Function_uint Function
// STORES-NOT: OpStore %[[S]]{{( |$)}}
// STORES: OpStore %[[S]] %uint_0{{( |$)}}
// STORES: OpStore %[[S]] {{%[^ ]+}}{{( |$)}}
// STORES-NOT: OpStore %[[S]]{{( |$)}}
// STORES: %[[INDEX:[^ ]+]] = OpLoad %uint %[[S]]{{( |$)}}
// STORES-NOT: OpStore %[[S]]{{( |$)}}
// STORES: %[[POINTER:[^ ]+]] = OpUntypedAccessChainKHR {{.*}} %slang_samplerHeap %[[INDEX]]{{( |$)}}
// STORES-NOT: OpStore %[[S]]{{( |$)}}
// STORES: %[[SAMPLER:[^ ]+]] = OpLoad {{[^ ]+}} %[[POINTER]]{{( |$)}}
// STORES-NOT: OpStore %[[S]]{{( |$)}}
// STORES: %[[PAIR:[^ ]+]] = OpSampledImage {{[^ ]+}} {{[^ ]+}} %[[SAMPLER]]{{( |$)}}
// STORES-NOT: OpStore %[[S]]{{( |$)}}
// STORES: OpImageSampleImplicitLod {{[^ ]+}} %[[PAIR]]{{( |$)}}
// STORES-NOT: OpStore %[[S]]{{( |$)}}

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
