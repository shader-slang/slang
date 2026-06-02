//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -stage fragment -entry fragmentMain
//TEST:SIMPLE(filecheck=CHECK_BINDLESS): -target spirv-asm -stage fragment -entry fragmentMain -capability spvBindlessTextureNV -DUSE_BINDLESS_NV

// A global DescriptorHandle constant lowers to a module-scope descriptor-handle
// representation cast. SPIR-V emission should treat that cast as transparent.
static const DescriptorHandle<SamplerState> kSampler =
    DescriptorHandle<SamplerState>(uint2(3, 4));
static const uint2 kSamplerBits = (uint2)kSampler;

#ifdef USE_BINDLESS_NV
static const DescriptorHandle<Texture2D<float4>> kTexture =
    DescriptorHandle<Texture2D<float4>>(uint64_t(5));
static const uint64_t kTextureBits = (uint64_t)kTexture;
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
    float2 uv = input.uv + float2(float(uint(kTextureBits)), 0.0) * 0.000001;
    return kTexture.Sample(kSampler, uv);
#else
    float2 uv = input.uv + float2(float(kSamplerBits.x), float(kSamplerBits.y)) * 0.000001;
    return args.texture.Sample(kSampler, uv);
#endif
}

// CHECK: OpConstantComposite
// CHECK: OpImageSampleImplicitLod

// CHECK_BINDLESS: OpCapability BindlessTextureNV
// CHECK_BINDLESS: OpConstant %ulong 5
// CHECK_BINDLESS: OpConvertUToSampledImageNV
// CHECK_BINDLESS: OpImageSampleImplicitLod
