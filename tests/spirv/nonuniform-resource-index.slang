// nonuniform-resource-index.slang

// Check that `NonUniformResourceIndex` on a descriptor array emits, for each
// resource class, (1) the NonUniform decoration on the operand the access
// consumes (the descriptor OpLoad for images/texel buffers, the OpAccessChain
// for uniform/storage buffers) and (2) the matching type-specific
// *ArrayNonUniformIndexing capability, alongside the base ShaderNonUniform.

//TEST:SIMPLE(filecheck=COMBINED):-target spirv -entry combinedSampler -stage fragment -emit-spirv-directly
//TEST:SIMPLE(filecheck=SAMPLED):-target spirv -entry sampledImage -stage fragment -emit-spirv-directly
//TEST:SIMPLE(filecheck=STOREIMG):-target spirv -entry storageImage -stage compute -emit-spirv-directly
//TEST:SIMPLE(filecheck=STOREBUF):-target spirv -entry storageBuffer -stage compute -emit-spirv-directly
//TEST:SIMPLE(filecheck=UNIFBUF):-target spirv -entry uniformBuffer -stage compute -emit-spirv-directly
//TEST:SIMPLE(filecheck=UNIFTEX):-target spirv -entry uniformTexelBuffer -stage compute -emit-spirv-directly
//TEST:SIMPLE(filecheck=STORETEX):-target spirv -entry storageTexelBuffer -stage compute -emit-spirv-directly

//
// --- combined sampler array (Sampler2D[] + .Sample) -----------------------
// Exact form of https://github.com/shader-slang/slang/issues/10930: the
// NonUniform decoration must reach the descriptor OpLoad consumed by
// OpImageSampleImplicitLod.
//
Sampler2D gCombined[];

// COMBINED: OpCapability SampledImageArrayNonUniformIndexing
// COMBINED: OpAccessChain {{.*}} %gCombined {{.*}}; NonUniform
// COMBINED: [[IMG:%[0-9a-zA-Z_]+]] = OpLoad {{.*}}; NonUniform
// COMBINED: OpImageSampleImplicitLod {{.*}} [[IMG]]
[shader("fragment")]
float4 combinedSampler(float2 uv : UV, uint i : I) : SV_Target
{
    return gCombined[NonUniformResourceIndex(i)].Sample(uv);
}

//
// --- sampled image array (Texture2D[] + separate SamplerState) ------------
//
Texture2D gTex[];
SamplerState gSamp;

// SAMPLED: OpCapability SampledImageArrayNonUniformIndexing
// SAMPLED: OpAccessChain {{.*}} %gTex {{.*}}; NonUniform
// SAMPLED: [[IMG:%[0-9a-zA-Z_]+]] = OpLoad {{.*}}; NonUniform
// SAMPLED: OpSampledImage {{.*}} [[IMG]]
[shader("fragment")]
float4 sampledImage(uint i : I) : SV_Target
{
    return gTex[NonUniformResourceIndex(i)].SampleLevel(gSamp, float2(0, 0), 0);
}

//
// --- storage image array (RWTexture2D[]) ----------------------------------
//
RWTexture2D<float4> gRWTex[];

// STOREIMG: OpCapability StorageImageArrayNonUniformIndexing
// STOREIMG: OpAccessChain {{.*}} %gRWTex {{.*}}; NonUniform
// STOREIMG: [[IMG:%[0-9a-zA-Z_]+]] = OpLoad {{.*}}; NonUniform
// STOREIMG: OpImageWrite [[IMG]]
[shader("compute")]
[numthreads(1, 1, 1)]
void storageImage(uint3 t : SV_DispatchThreadID)
{
    gRWTex[NonUniformResourceIndex(t.x)][uint2(0, 0)] = float4(1, 0, 0, 0);
}

//
// --- storage buffer array (RWStructuredBuffer[]) --------------------------
//
RWStructuredBuffer<float4> gSB[];

// STOREBUF: OpCapability StorageBufferArrayNonUniformIndexing
// STOREBUF: OpAccessChain {{.*}} %gSB {{.*}}; NonUniform
[shader("compute")]
[numthreads(1, 1, 1)]
void storageBuffer(uint3 t : SV_DispatchThreadID)
{
    gSB[NonUniformResourceIndex(t.x)][0] = float4(2, 0, 0, 0);
}

//
// --- uniform buffer array (ConstantBuffer[]) ------------------------------
//
struct Foo { float4 v; };
ConstantBuffer<Foo> gCB[];
RWStructuredBuffer<float4> gOutCB;

// UNIFBUF: OpCapability UniformBufferArrayNonUniformIndexing
// UNIFBUF: OpAccessChain {{.*}} %gCB {{.*}}; NonUniform
[shader("compute")]
[numthreads(1, 1, 1)]
void uniformBuffer(uint3 t : SV_DispatchThreadID)
{
    gOutCB[0] = gCB[NonUniformResourceIndex(t.x)].v;
}

//
// --- uniform texel buffer array (Buffer<T>[]) -----------------------------
//
Buffer<float4> gTB[];
RWStructuredBuffer<float4> gOutTB;

// UNIFTEX: OpCapability UniformTexelBufferArrayNonUniformIndexing
// UNIFTEX: OpAccessChain {{.*}} %gTB {{.*}}; NonUniform
// UNIFTEX: [[IMG:%[0-9a-zA-Z_]+]] = OpLoad {{.*}}; NonUniform
// UNIFTEX: OpImageFetch {{.*}} [[IMG]]
[shader("compute")]
[numthreads(1, 1, 1)]
void uniformTexelBuffer(uint3 t : SV_DispatchThreadID)
{
    gOutTB[0] = gTB[NonUniformResourceIndex(t.x)].Load(0);
}

//
// --- storage texel buffer array (RWBuffer<T>[]) ---------------------------
//
RWBuffer<float4> gRWB[];

// STORETEX: OpCapability StorageTexelBufferArrayNonUniformIndexing
// STORETEX: OpAccessChain {{.*}} %gRWB {{.*}}; NonUniform
// STORETEX: [[IMG:%[0-9a-zA-Z_]+]] = OpLoad {{.*}}; NonUniform
// STORETEX: OpImageWrite [[IMG]]
[shader("compute")]
[numthreads(1, 1, 1)]
void storageTexelBuffer(uint3 t : SV_DispatchThreadID)
{
    gRWB[NonUniformResourceIndex(t.x)][0] = float4(3, 0, 0, 0);
}
