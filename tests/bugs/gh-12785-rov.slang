// Regression test for shader-slang/slang#12785 (rasterizer-ordered views are conservatively excluded).
//
// The CSE added for #12785 commons a dominated identical call only when the callee reads exclusively
// IMMUTABLE (read-only SRV) memory. A rasterizer-ordered view is a read/write resource, so it is not
// in the immutable allowlist and a wrapper reading one is not treated as a repeatable read-only
// access -- its calls are not commoned. (A single-invocation ROV reuse with no intervening write is
// arguably sound since ROV only serializes across invocations, so this is a deliberate conservative
// exclusion rather than a strict correctness requirement; the positive gate excludes it for free.)

//TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry computeMain -stage compute

RasterizerOrderedStructuredBuffer<uint> rov;
RWStructuredBuffer<uint> outputData;

[noinline]
uint loadRov(uint index)
{
    return rov[index];
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint index = dispatchThreadID.x;
    uint a = loadRov(index);
    uint b = loadRov(index);
    outputData[index] = a + b;
}
// Both calls survive (the definition reads `loadRov_0(uint ...`, not `loadRov_0(index`).
// CHECK-COUNT-2: loadRov_0(index
// CHECK-NOT: loadRov_0(index
