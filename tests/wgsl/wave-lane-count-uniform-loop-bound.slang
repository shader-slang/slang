//TEST:SIMPLE(filecheck=CHECK): -target wgsl -entry computeMain -stage compute
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-wgpu -compute -shaderobj -output-using-type -g0
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-vk -compute -shaderobj -output-using-type

// WGSL's uniformity analysis treats every read of a module-scope `var<private>` as possibly
// non-uniform. On WGSL, `translateGlobalVaryingVar` stores `@builtin(subgroup_size)` into a
// private shadow copy at the top of the entry point, and the inlined `WaveGetLaneCount()` emits
// `__requireTargetExtension("subgroups")` between that store and its load of the copy. The loop
// bound reads the builtin parameter directly, which `u32(256) / [[LC]]` checks, only when
// store-to-load forwarding crosses the requirement; otherwise Tint rejects the `subgroupAdd` in
// the loop as not in subgroup-uniform control flow (shader-slang/slang#8774). The `-wgpu` row
// runs the shader through Tint. Forwarding still stops at a block boundary, at any side-effecting
// call between the store and the load (including every wave op) and at debug insts
// (`-g`/`-g2`/`-g3`), so those shapes still read the `var<private>`. render-test emits debug info
// by default, so the `-wgpu` row passes `-g0`.

// CHECK: enable subgroups;
// CHECK: fn computeMain(
// CHECK-SAME: @builtin(subgroup_size) [[LC:[A-Za-z0-9_]+]] : u32
// CHECK: u32(256) / [[LC]];
// CHECK: subgroupAdd(

static const uint32_t kWgSize = 256;

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[shader("compute")]
[numthreads(kWgSize, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    let subgroupSize = WaveGetLaneCount();
    let numSubgroups = kWgSize / subgroupSize;

    // Each iteration adds the full subgroup's lane count, so the total is kWgSize whatever
    // the subgroup size is.
    uint total = 0;
    for (uint i = 0; i < numSubgroups; ++i)
    {
        total += WaveActiveSum(1u);
    }

    if (tid.x == 0)
        outputBuffer[0] = total;
}

// BUF: 256
