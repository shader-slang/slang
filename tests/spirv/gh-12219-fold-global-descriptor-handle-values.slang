//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry computeMain -stage compute

// Compile-only check that the `DescriptorHandle` round-trip cancellation preserves the bit-lane
// order, for both a same-width round trip and a `bit_cast`-bridged one. Element 0 is the low bits:
// `bit_cast<uint2>(0x0000000400000003)` is `{3, 4}`, matching the runtime `OpBitcast` lane order.

// Same-width: (uint2)(DescriptorHandle)uint2(3, 4) cancels to the uint2 constant {3, 4}.
static const DescriptorHandle<RWStructuredBuffer<uint2>> gSameHandle =
    (DescriptorHandle<RWStructuredBuffer<uint2>>)uint2(3u, 4u);
static const uint2 gSame = (uint2)gSameHandle;

// bit_cast-bridged: the uint64 0x0000000400000003 reinterprets to the same {3, 4} lanes.
static const DescriptorHandle<RWStructuredBuffer<uint2>> gBridgedHandle =
    (DescriptorHandle<RWStructuredBuffer<uint2>>)bit_cast<uint2>(0x0000000400000003ull);
static const uint2 gBridged = (uint2)gBridgedHandle;

RWStructuredBuffer<uint2> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    // No representation cast survives either round trip.
    // CHECK-NOT: DescriptorHandle
    // Same-width folds to the uint2 constant {3, 4}.
    // CHECK-DAG: %[[U3:[A-Za-z0-9_]+]] = OpConstant %uint 3
    // CHECK-DAG: %[[U4:[A-Za-z0-9_]+]] = OpConstant %uint 4
    // CHECK-DAG: OpConstantComposite %v2uint %[[U3]] %[[U4]]
    // bit_cast-bridged keeps the packed uint64 (0x0000000400000003 = 17179869187), reinterpreted to
    // uint2 by an in-function OpBitcast.
    // CHECK-DAG: %[[BITS:[A-Za-z0-9_]+]] = OpConstant %ulong 17179869187
    // CHECK: OpBitcast %v2uint %[[BITS]]
    outputBuffer[0] = gSame;
    outputBuffer[1] = gBridged;
}
