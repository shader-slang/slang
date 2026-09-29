//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry computeMain -stage compute

// Compile-only check that the `DescriptorHandle` round-trip cancellation preserves the bit-lane
// order, for two `uint2` initializers: a direct vector and a `bit_cast` from a `uint64`. Element 0
// is the low bits: `bit_cast<uint2>(0x0000000400000003)` is `{3, 4}`, matching the runtime
// `OpBitcast` lane order.

// Direct vector: (uint2)(DescriptorHandle)uint2(3, 4) cancels to the uint2 constant {3, 4}.
static const DescriptorHandle<RWStructuredBuffer<uint2>> gDirectHandle =
    (DescriptorHandle<RWStructuredBuffer<uint2>>)uint2(3u, 4u);
static const uint2 gDirect = (uint2)gDirectHandle;

// bit_cast source: the uint64 0x0000000400000003 reinterprets to the same {3, 4} lanes.
static const DescriptorHandle<RWStructuredBuffer<uint2>> gBitCastHandle =
    (DescriptorHandle<RWStructuredBuffer<uint2>>)bit_cast<uint2>(0x0000000400000003ull);
static const uint2 gBitCast = (uint2)gBitCastHandle;

RWStructuredBuffer<uint2> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    // No representation cast survives either round trip.
    // CHECK-NOT: DescriptorHandle
    // The direct vector folds to the uint2 constant {3, 4}.
    // CHECK-DAG: %[[U3:[A-Za-z0-9_]+]] = OpConstant %uint 3
    // CHECK-DAG: %[[U4:[A-Za-z0-9_]+]] = OpConstant %uint 4
    // CHECK-DAG: OpConstantComposite %v2uint %[[U3]] %[[U4]]
    // The bit_cast source keeps the packed uint64 (0x0000000400000003 = 17179869187), reinterpreted
    // to uint2 by an in-function OpBitcast.
    // CHECK-DAG: %[[BITS:[A-Za-z0-9_]+]] = OpConstant %ulong 17179869187
    // CHECK: OpBitcast %v2uint %[[BITS]]
    outputBuffer[0] = gDirect;
    outputBuffer[1] = gBitCast;
}
