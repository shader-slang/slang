// A single-use immutable load must fold into the expression that uses it
// even outside a short-circuit: the `__ldg` read appears inside the emitted
// assignment, not as a preceding statement. Pins the general effect of the
// synthesized-helper purity decorations. (Companion to
// shader-slang/slang#14494.)

//TEST:SIMPLE(filecheck=CHECK): -stage compute -entry computeMain -target cuda

RWStructuredBuffer<uint> output;

StructuredBuffer<float3> itemMin;

[shader("compute")]
[numthreads(4, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    //CHECK-NOT: = slang_ldg_0
    output[tid.x] = uint(itemMin[tid.x].x + 1.0);
    //CHECK: = {{.*}}slang_ldg_0
}
