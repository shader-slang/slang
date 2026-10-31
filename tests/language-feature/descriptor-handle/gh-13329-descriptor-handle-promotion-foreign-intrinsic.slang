// Laying out a `DescriptorHandle` on a target with no profile or capability promotes the
// `descriptor_handle` capability into the target. The promotion must not make another target's
// `__target_intrinsic` selectable: `pick` has only a Metal intrinsic, so HLSL, WGSL and C++ must
// call its body. See shader-slang/slang#13329.

//TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK): -target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK): -target cpp -entry computeMain -stage compute

// CHECK-NOT: METAL_ONLY
// CHECK: pick_0(
// CHECK-NOT: METAL_ONLY

__target_intrinsic(metal, "METAL_ONLY($0)")
uint pick(uint x)
{
    return x + 7;
}

uniform DescriptorHandle<RWStructuredBuffer<uint>> gOut;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    gOut[0] = pick(tid.x);
}
