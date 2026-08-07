//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry computeMain -stage compute

// A module-scope `static const` DescriptorHandle initialized from a constant integer `bit_cast`
// used to abort SPIR-V emit. The `DescriptorHandle` representation-cast round-trip
// (`CastDescriptorHandleToUInt2(CastUInt2ToDescriptorHandle(bit_cast<uint2>(uint64)))`) left the
// scalar->vector `bit_cast` alive at global scope, where a constant `OpBitcast` is invalid SPIR-V
// and `emitGlobalInst` had no handler (E99997). The new peephole cancels the round trip so the
// `bit_cast` moves into the entry point where it is valid (and higher optimization levels then fold
// it to a constant composite). The constant must be observably consumed (stored to a buffer) or
// dead-code elimination drops it before emit and masks the bug.

static const DescriptorHandle<RWStructuredBuffer<uint2>> gHandle =
    (DescriptorHandle<RWStructuredBuffer<uint2>>)bit_cast<uint2>(0x0000000400000003ull);
static const uint2 gBits = (uint2)gHandle;

RWStructuredBuffer<uint2> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    // The DescriptorHandle round trip is cancelled, so no representation cast survives and the value
    // reaches the store as the original uint64 bit pattern (0x0000000400000003), reinterpreted to
    // uint2 by an in-function `ulong`->`uint2` scalar-to-vector OpBitcast (valid SPIR-V) rather than
    // the invalid global-scope one that aborted before the fix.
    // CHECK-NOT: DescriptorHandle
    // CHECK-DAG: %[[BITS:[A-Za-z0-9_]+]] = OpConstant %ulong 17179869187
    // CHECK: OpBitcast %v2uint %[[BITS]]
    outputBuffer[0] = gBits;
}
