//TEST:SIMPLE(filecheck=SAMPLER): -target spirv -capability spvBindlessTextureNV -stage compute -entry computeMain -DSAMPLER
//TEST:SIMPLE(filecheck=COMBINED_IMAGE_SAMPLER): -target spirv -capability spvBindlessTextureNV -stage compute -entry computeMain -DCOMBINED_IMAGE_SAMPLER
//TEST:SIMPLE(filecheck=SAMPLED_IMAGE): -target spirv -capability spvBindlessTextureNV -stage compute -entry computeMain -DSAMPLED_IMAGE
//TEST:SIMPLE(filecheck=STORAGE_IMAGE): -target spirv -capability spvBindlessTextureNV -stage compute -entry computeMain -DSTORAGE_IMAGE

[[vk::binding(0)]] RWTexture1D<float> t1;
[[vk::binding(1)]] RWTexture1D<float> t2;
[[vk::binding(2)]] RWTexture1D<float> t3;
[[vk::binding(3)]] Texture1D<float> t4;

#ifdef SAMPLER
//SAMPLER: [[SType:%[0-9]+]] = OpTypeSampler
//SAMPLER: OpConvertUToSamplerNV [[SType]]
//SAMPLER: OpSampledImage
uniform SamplerState.Handle sampler;
#endif

#ifdef COMBINED_IMAGE_SAMPLER
//COMBINED_IMAGE_SAMPLER: [[SIType:%[0-9]+]] = OpTypeSampledImage
//COMBINED_IMAGE_SAMPLER: OpConvertUToSampledImageNV [[SIType]]
uniform Sampler1DShadow.Handle combinedSampler;
#endif

#ifdef SAMPLED_IMAGE
//SAMPLED_IMAGE: [[IType0:%[0-9]+]] = OpTypeImage
//SAMPLED_IMAGE: [[IType1:%[0-9]+]] = OpTypeImage
//SAMPLED_IMAGE: OpConvertUToSampledImageNV [[IType1]]
//SAMPLED_IMAGE: OpImageFetch
uniform Texture1D<float>.Handle texture;
#endif

#ifdef STORAGE_IMAGE
//STORAGE_IMAGE: OpConvertUToSampledImageNV
//STORAGE_IMAGE: OpImageRead
//STORAGE_IMAGE: OpConvertUToSampledImageNV
//STORAGE_IMAGE: OpImageRead
uniform RWTexture1D<float>.Handle rwTexture1;
uniform RWTexture2D<float>.Handle rwTexture2;
#endif

[shader("compute")]
[numthreads(8, 8, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    t1[0] = t2[0] + t2[0] + t4[0];

#ifdef SAMPLER
    t1[2] = t4.Sample(sampler, 0);
#endif

#ifdef COMBINED_IMAGE_SAMPLER
    t1[8] = combinedSampler.Sample(0);
#endif

#ifdef SAMPLED_IMAGE
    t1[0] = texture[0];
#endif

#ifdef STORAGE_IMAGE
    t1[11] = rwTexture1[0];
    t1[12] = rwTexture2[0];
#endif
}

