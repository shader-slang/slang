//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -stage fragment -entry fragmentMain

// A global DescriptorHandle constant lowers to a module-scope descriptor-handle
// representation cast. SPIR-V emission should treat that cast as transparent.
static const DescriptorHandle<SamplerState> kSampler =
    DescriptorHandle<SamplerState>(uint2(0, 0));

struct FragmentInput
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD0;
};

struct Args
{
    DescriptorHandle<Texture2D<float4>> texture;
};

[[vk::push_constant]] uniform Args args;

[shader("fragment")]
float4 fragmentMain(FragmentInput input) : SV_Target
{
    return args.texture.Sample(kSampler, input.uv);
}

// CHECK: OpImageSampleImplicitLod
