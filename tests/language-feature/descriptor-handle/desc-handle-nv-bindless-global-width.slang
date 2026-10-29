// Unoptimized: the initializer must be sunk into the function, so the width bridge appears as
// in-block instructions. The second directive emits binary SPIR-V, to a file so stdout stays empty,
// which validates the sunk form.
//TEST:SIMPLE(filecheck=CHECK_O0): -target spirv-asm -capability spvBindlessTextureNV -stage compute -entry computeMain -O0
//TEST:SIMPLE: -target spirv -capability spvBindlessTextureNV -stage compute -entry computeMain -O0 -o desc-handle-nv-bindless-global-width.spv

// Optimized: the same program folds to correctly-typed constants and the bridge disappears.
//TEST:SIMPLE(filecheck=CHECK_O1): -target spirv-asm -capability spvBindlessTextureNV -stage compute -entry computeMain -O1

// A `DescriptorHandle`'s SPIR-V representation is `uint64` for the kinds `spvBindlessTextureNV` can
// convert and `uint2` for the rest, and either integer constructor is available for any kind. A
// global constant can therefore be built at one width and read at the other. The four representation
// casts are inlinable global insts, so such an initializer sinks into the function that uses it,
// where the widths reconcile — rather than surviving at module scope, where it would feed a
// wrong-width value to arithmetic (and where a bridging `OpBitcast` is not valid SPIR-V).

// A `uint2`-initialized sampler handle (representation `uint64`) read back as `uint64_t`, reached
// through a compile-time `?:` so the inlined chain also carries a `Select`. The taken branch packs
// to `(4 << 32) | 3`.
static const DescriptorHandle<SamplerState> kSampler =
    DescriptorHandle<SamplerState>(true ? uint2(3, 4) : uint2(5, 6));
static const uint64_t kSamplerBits = (uint64_t)kSampler;

// A `uint64`-initialized buffer handle (representation `uint2`) dereferenced through the descriptor
// heap. The heap index is the initializer's low word.
static const DescriptorHandle<RWStructuredBuffer<float> > kBuffer =
    DescriptorHandle<RWStructuredBuffer<float> >(uint64_t(0x0000000400000003));

[[vk::binding(0)]] RWStructuredBuffer<uint64_t> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    outputBuffer[0] = kSamplerBits + tid.x;

    RWStructuredBuffer<float> b = kBuffer;
    outputBuffer[1] = (uint64_t)b[0];
}

// At -O0 both bridges live inside the entry point's block, after its label: the `uint2` initializer
// selects a vector and is bitcast up to `%ulong` for the arithmetic, and the `uint64` initializer is
// bitcast down to `%v2uint` before its low word indexes the heap.
//CHECK_O0: OpLabel
//CHECK_O0-DAG: %[[SEL:[0-9A-Za-z_]+]] = OpSelect %v2uint
//CHECK_O0-DAG: %[[UP:[0-9A-Za-z_]+]] = OpBitcast %ulong %[[SEL]]
//CHECK_O0-DAG: OpIAdd %ulong %[[UP]]
//CHECK_O0-DAG: %[[DOWN:[0-9A-Za-z_]+]] = OpBitcast %v2uint %{{[0-9A-Za-z_]+}}
//CHECK_O0-DAG: %[[LOW:[0-9A-Za-z_]+]] = OpCompositeExtract %uint %[[DOWN]] 0
//CHECK_O0-DAG: OpAccessChain {{%[0-9A-Za-z_]+}} %__slang_resource_heap %[[LOW]]

// At -O1 the select and both bitcasts fold away, leaving a `%ulong` constant as the addition's
// operand and a `%uint` constant as the heap index. A surviving wrong-width value would show up
// here as a `%v2uint` operand to the `OpIAdd`, or as an extract from a `%ulong`.
//CHECK_O1-DAG: %[[BITS:[0-9A-Za-z_]+]] = OpConstant %ulong 17179869187
//CHECK_O1-DAG: %[[INDEX:[0-9A-Za-z_]+]] = OpConstant %uint 3
// Each `-NOT` sits between two positive matches so it is bounded to that span; together they cover
// the whole function body, where the folded-away bridge would otherwise reappear.
//CHECK_O1: OpLabel
//CHECK_O1-NOT: OpBitcast
//CHECK_O1: OpIAdd %ulong %[[BITS]]
//CHECK_O1-NOT: OpBitcast
//CHECK_O1: OpAccessChain {{%[0-9A-Za-z_]+}} %__slang_resource_heap %[[INDEX]]
//CHECK_O1-NOT: OpBitcast
//CHECK_O1: OpFunctionEnd
