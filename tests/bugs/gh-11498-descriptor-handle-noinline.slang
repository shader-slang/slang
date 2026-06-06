//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -profile spirv_1_6 -capability spvDescriptorHeapEXT -stage fragment -entry main

// Regression test for shader-slang/slang#11498.
// A `[noinline]` callee that takes a Texture2D parameter and is called with
// `DescriptorHandle<Texture2D>` under -capability spvDescriptorHeapEXT used to
// crash mid-emit (orphan IRParam reaching emitSpvAsmOperand). The texture
// argument lowers to kIROp_SPIRVLoadDescriptorFromHeap, which the function-call
// specialization allowlists did not accept, leaving the texture parameter
// unspecialized and its asm-operand back-reference desynced by later passes.

// CHECK: OpImageSampleImplicitLod

SamplerState sampler;
static const uint N = 2;

[noinline]
float4 bad_function(Texture2D tex, float4 values[N])
{
    return tex.Sample(sampler, float2(0.0));
}

struct Data { float4 values[N]; };
struct PushConstants {
    DescriptorHandle<Texture2D> tex;
    ImmutablePtr<Data> data;
};

[shader("fragment")]
float4 main(uniform PushConstants pc) : SV_Target
{
    return bad_function(pc.tex, pc.data->values);
}
