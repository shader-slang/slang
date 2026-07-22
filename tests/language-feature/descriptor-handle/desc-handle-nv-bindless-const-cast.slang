//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -capability spvBindlessTextureNV -stage compute -entry computeMain

// A module-scope `static const DescriptorHandle` whose stored width differs from the width its
// element kind uses under `spvBindlessTextureNV`, then read back as an integer of the kind's width,
// must produce a correctly-typed constant — not a `uint2` value used where a `uint64` is required,
// and not a constant `OpBitcast` (which is invalid SPIR-V). Here `kTexture` is a texture handle
// (uint64 representation) built from a `uint2` literal, read back as `uint64_t` and consumed by an
// ordinary `uint64` add, so the reinterpreted constant must be `%ulong`. The `?:` initializers also
// exercise reinterpreting a constant that reaches the cast through a compile-time `select`. See
// shader-slang/slang#12185. (Regression companion to tests/bugs/gh-9916.slang.)

// `true` branch taken: uint2(3, 4) -> (4 << 32) | 3 == 17179869187.
static const DescriptorHandle<Texture2D<float4> > kTextureA =
    DescriptorHandle<Texture2D<float4> >(true ? uint2(3, 4) : uint2(5, 6));
static const uint64_t kTextureBitsA = (uint64_t)kTextureA;

// `false` branch taken: uint2(5, 6) -> (6 << 32) | 5 == 25769803781.
static const DescriptorHandle<Texture2D<float4> > kTextureB =
    DescriptorHandle<Texture2D<float4> >(false ? uint2(3, 4) : uint2(5, 6));
static const uint64_t kTextureBitsB = (uint64_t)kTextureB;

[[vk::binding(0)]] RWStructuredBuffer<uint64_t> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    //CHECK-DAG: %{{[0-9A-Za-z_]+}} = OpConstant %ulong 17179869187
    //CHECK-DAG: %{{[0-9A-Za-z_]+}} = OpConstant %ulong 25769803781
    //CHECK: OpIAdd %ulong
    // Separate stores so each reinterpreted constant survives independently (adding a per-thread
    // value keeps the store from folding the handle bits away).
    outputBuffer[0] = kTextureBitsA + tid.x;
    outputBuffer[1] = kTextureBitsB + tid.y;
}
