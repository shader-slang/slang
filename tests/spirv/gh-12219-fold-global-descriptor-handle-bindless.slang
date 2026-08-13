//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry computeMain -stage compute -capability spvBindlessTextureNV

// Same #12219 regression as gh-12219-fold-global-descriptor-handle-const.slang, but exercising the
// peephole's `uint64` unwrap branch: under `spvBindlessTextureNV` a texture `DescriptorHandle` is
// represented as `uint64`, so initializing one from `bit_cast<uint64_t>(uint2(...))` produces the
// chain `CastDescriptorHandleToUInt64(CastUInt64ToDescriptorHandle(bit_cast<uint64>(...)))`. The
// `bit_cast` between the casts and the consumer is what survives to global-scope emit and aborts
// (E99997) without the peephole — the same failure mode as the `uint2` test, on the `uint64` branch.
// (A same-width uint64 round trip with no `bit_cast` already emits fine via the transparent-cast
// forwarding that tests/bugs/gh-9916.slang covers, so this uses the `bit_cast`-in-chain shape.)

static const DescriptorHandle<Texture2D<float4>> gTexture =
    DescriptorHandle<Texture2D<float4>>(bit_cast<uint64_t>(uint2(3u, 4u)));
static const uint64_t gBits = (uint64_t)gTexture;

RWStructuredBuffer<uint64_t> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    // The round trip is cancelled, so no representation cast survives; the re-exposed
    // `bit_cast<uint64>(uint2(3, 4))` reaches the store as an in-function `OpBitcast` to `%ulong`
    // (valid SPIR-V), rather than an invalid global-scope one.
    // CHECK-NOT: DescriptorHandle
    // CHECK: OpBitcast %ulong
    outputBuffer[0] = gBits;
}
