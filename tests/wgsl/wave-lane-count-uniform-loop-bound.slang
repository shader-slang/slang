//TEST:SIMPLE(filecheck=CHECK): -target wgsl -entry computeMain -stage compute
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-wgpu -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-vk -compute -shaderobj -output-using-type
//TEST:SIMPLE(filecheck=FRAG): -target wgsl -entry fragmentMain -stage fragment

// WGSL's uniformity analysis treats every read of a module-scope `var<private>` as possibly
// non-uniform. On WGSL, `translateGlobalVaryingVar` stores `@builtin(subgroup_size)` into a
// private shadow copy at the top of the entry point, and the inlined `WaveGetLaneCount()` emits
// `__requireTargetExtension("subgroups")` between that store and its load of the copy. A read of
// the copy would make Tint reject the `subgroupAdd` in the loop as not in subgroup-uniform control
// flow (shader-slang/slang#8774).
//
// Each row pins a different fix. In a compute shader, `introduceExplicitGlobalContext` replaces
// the shadow with a local copy, so the compute CHECK lines and the `-wgpu` row, which runs the
// shader through Tint, pass whether or not store-to-load forwarding runs. The fragment row pins
// the forwarding: the lane count is not uniform in a fragment shader, so its shadow stays a
// `var<private>`, and `u32(256) / [[FLC]]` reads the builtin parameter only because the store is
// forwarded across the requirement.

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

[shader("fragment")]
float4 fragmentMain() : SV_Target
{
    let laneCount = WaveGetLaneCount();
    return float4(float(256 / laneCount), 0, 0, 1);
}

// FRAG: fn fragmentMain(
// FRAG-SAME: @builtin(subgroup_size) [[FLC:[A-Za-z0-9_]+]] : u32
// FRAG: u32(256) / [[FLC]]
