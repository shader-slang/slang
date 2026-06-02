//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -stage fragment -entry fragmentMain
//TEST:SIMPLE(filecheck=CHECK_BINDLESS): -target spirv-asm -stage fragment -entry fragmentMain -capability spvBindlessTextureNV -DUSE_BINDLESS_NV

// A global DescriptorHandle constant lowers to a module-scope descriptor-handle
// representation cast. SPIR-V emission should treat that cast as transparent.
static const DescriptorHandle<SamplerState> kSampler =
    DescriptorHandle<SamplerState>(uint2(0, 0));

#ifdef USE_BINDLESS_NV
static const DescriptorHandle<Texture2D<float4>> kTexture =
    DescriptorHandle<Texture2D<float4>>(uint64_t(0));
#endif

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
#ifdef USE_BINDLESS_NV
    return kTexture.Sample(kSampler, input.uv);
#else
    return args.texture.Sample(kSampler, input.uv);
#endif
}

// CHECK: OpImageSampleImplicitLod

// CHECK_BINDLESS: OpCapability BindlessTextureNV
// CHECK_BINDLESS: OpConvertUToSampledImageNV
// CHECK_BINDLESS: OpImageSampleImplicitLod
