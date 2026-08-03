//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -capability spvBindlessTextureNV -stage compute -entry computeMain

// A `DescriptorHandle`'s SPIR-V representation is `uint64` for the kinds `spvBindlessTextureNV` can
// convert and `uint2` for the rest, and either integer constructor is available for any kind. A
// global constant can therefore be built at one width and read at the other. Since the four
// representation casts are inlinable global insts, such an initializer sinks into the function that
// uses it, where the widths reconcile — rather than surviving at module scope, where it would feed a
// wrong-width value to arithmetic (and where a bridging `OpBitcast` is not valid SPIR-V).

// A `uint2`-initialized sampler handle (whose representation is `uint64`) read back as `uint64_t`,
// then used in `uint64` arithmetic. The packed constant is `(4 << 32) | 3`.
//CHECK-DAG: %[[BITS:[0-9A-Za-z_]+]] = OpConstant %ulong 17179869187
static const DescriptorHandle<SamplerState> kSampler =
    DescriptorHandle<SamplerState>(uint2(3, 4));
static const uint64_t kSamplerBits = (uint64_t)kSampler;

// A `uint64`-initialized buffer handle (whose representation is `uint2`) dereferenced through the
// descriptor heap. The heap index is the initializer's low word.
//CHECK-DAG: %[[INDEX:[0-9A-Za-z_]+]] = OpConstant %uint 3
static const DescriptorHandle<RWStructuredBuffer<float> > kBuffer =
    DescriptorHandle<RWStructuredBuffer<float> >(uint64_t(0x0000000400000003));

[[vk::binding(0)]] RWStructuredBuffer<uint64_t> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    // The addition takes the reinterpreted constant itself as an operand: if the `uint2` value
    // survived instead, this would be a `%v2uint` operand and the module would fail validation.
    //CHECK: OpIAdd %ulong %[[BITS]]
    outputBuffer[0] = kSamplerBits + tid.x;

    // The heap index is the folded `%uint` constant. If the `uint64` value survived, this operand
    // would instead be an `OpCompositeExtract` whose composite is a `%ulong`.
    //CHECK: OpAccessChain {{%[0-9A-Za-z_]+}} %__slang_resource_heap %[[INDEX]]
    RWStructuredBuffer<float> b = kBuffer;
    outputBuffer[1] = (uint64_t)b[0];
}
