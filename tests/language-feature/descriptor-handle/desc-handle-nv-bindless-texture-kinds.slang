// Under `spvBindlessTextureNV`, a `DescriptorHandle<T>` is converted with the extension's dedicated
// opcode only for the image/sampler family — the kinds whose handle is represented as `uint64`:
// textures and texel buffers via `OpConvertUToImageNV`, combined texture-samplers via
// `OpConvertUToSampledImageNV`, samplers and comparison samplers via `OpConvertUToSamplerNV`.
// Every other kind keeps the `uint2` representation and the ordinary dynamic-resource-heap path:
// constant/structured/byte-address buffers index the heap (no NV opcode, no compiler abort), and
// acceleration structures use `OpConvertUToAccelerationStructureKHR`. See shader-slang/slang#12185.

//TEST:SIMPLE(filecheck=AS): -target spirv -capability spvBindlessTextureNV -stage compute -entry computeMain -DACCELERATION_STRUCTURE
//TEST:SIMPLE(filecheck=TEXEL_BUFFER): -target spirv -capability spvBindlessTextureNV -stage compute -entry computeMain -DTEXEL_BUFFER
//TEST:SIMPLE(filecheck=SAMPLER_COMPARISON): -target spirv -capability spvBindlessTextureNV -stage compute -entry computeMain -DSAMPLER_COMPARISON
//TEST:SIMPLE(filecheck=CONSTANT_BUFFER): -target spirv -capability spvBindlessTextureNV -stage compute -entry computeMain -DCONSTANT_BUFFER
//TEST:SIMPLE(filecheck=STORAGE_BUFFER): -target spirv -capability spvBindlessTextureNV -stage compute -entry computeMain -DSTORAGE_BUFFER
//TEST:SIMPLE(filecheck=BYTE_ADDRESS_BUFFER): -target spirv -capability spvBindlessTextureNV -stage compute -entry computeMain -DBYTE_ADDRESS_BUFFER

[[vk::binding(0)]] RWStructuredBuffer<float> outputBuffer;
[[vk::binding(1)]] Texture2D<float> gTexture;

#ifdef ACCELERATION_STRUCTURE
//AS: OpConvertUToAccelerationStructureKHR
uniform RaytracingAccelerationStructure.Handle rayAccel;
#endif

#ifdef TEXEL_BUFFER
//TEXEL_BUFFER: OpConvertUToImageNV
uniform Buffer<float>.Handle texelBuffer;
#endif

#ifdef SAMPLER_COMPARISON
//SAMPLER_COMPARISON: OpConvertUToSamplerNV
uniform SamplerComparisonState.Handle cmpSampler;
#endif

// The buffer kinds the NV extension cannot encode must still compile: they keep the `uint2` handle
// and index the dynamic-resource heap, so the module is valid and emits neither an NV conversion
// opcode nor an abort.

#ifdef CONSTANT_BUFFER
//CONSTANT_BUFFER-NOT: OpConvertUToImageNV
//CONSTANT_BUFFER-NOT: OpConvertUToSamplerNV
//CONSTANT_BUFFER: OpEntryPoint
struct Data { float v; }
uniform ConstantBuffer<Data>.Handle cbuffer;
#endif

#ifdef STORAGE_BUFFER
//STORAGE_BUFFER-NOT: OpConvertUToImageNV
//STORAGE_BUFFER-NOT: OpConvertUToSamplerNV
//STORAGE_BUFFER: OpEntryPoint
uniform RWStructuredBuffer<float>.Handle sbuffer;
#endif

#ifdef BYTE_ADDRESS_BUFFER
//BYTE_ADDRESS_BUFFER-NOT: OpConvertUToImageNV
//BYTE_ADDRESS_BUFFER-NOT: OpConvertUToSamplerNV
//BYTE_ADDRESS_BUFFER: OpEntryPoint
uniform ByteAddressBuffer.Handle babuffer;
#endif

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
#ifdef ACCELERATION_STRUCTURE
    RayDesc ray;
    ray.Origin = float3(0.0f, 0.0f, 0.0f);
    ray.Direction = float3(0.0f, 0.0f, 1.0f);
    ray.TMin = 0.0f;
    ray.TMax = 100.0f;
    RayQuery<RAY_FLAG_FORCE_NON_OPAQUE> rq;
    rq.TraceRayInline(rayAccel, RAY_FLAG_FORCE_NON_OPAQUE, 0xff, ray);
    outputBuffer[0] = (float)rq.RayFlags();
#endif

#ifdef TEXEL_BUFFER
    outputBuffer[0] = texelBuffer[0];
#endif

#ifdef SAMPLER_COMPARISON
    outputBuffer[0] = gTexture.SampleCmpLevelZero(cmpSampler, float2(0.0f, 0.0f), 0.5f);
#endif

#ifdef CONSTANT_BUFFER
    outputBuffer[0] = (*cbuffer).v;
#endif

#ifdef STORAGE_BUFFER
    outputBuffer[0] = sbuffer[0];
#endif

#ifdef BYTE_ADDRESS_BUFFER
    outputBuffer[0] = (float)babuffer.Load<uint>(0);
#endif
}
