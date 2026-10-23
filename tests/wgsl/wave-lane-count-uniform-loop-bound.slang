//TEST:SIMPLE(filecheck=CHECK): -target wgsl -entry main -stage compute

// WGSL's uniformity analysis treats every read of a module-scope `var<private>` as possibly
// non-uniform. On WGSL, `WaveGetLaneCount()` reads the private shadow copy of
// `@builtin(subgroup_size)` that `translateGlobalVaryingVar` creates, after a
// `__requireTargetExtension("subgroups")`. The loop bound reads the builtin parameter directly
// only when store-to-load forwarding crosses that requirement; otherwise Tint rejects the
// `subgroupAdd` in the loop as not in subgroup-uniform control flow (shader-slang/slang#8774).

// CHECK: enable subgroups;
// CHECK: fn main(@builtin(subgroup_size) [[LC:[A-Za-z0-9_]+]] : u32)
// CHECK: u32(256) / [[LC]];
// CHECK: subgroupAdd(

static const uint32_t kWgSize = 256;

RWStructuredBuffer<int> outputBuffer;

[shader("compute")]
[numthreads(kWgSize, 1, 1)]
void main()
{
    let subgroupSize = WaveGetLaneCount();
    let numSubgroups = kWgSize / subgroupSize;

    var testValue = 1;

    for (var i = 0; i < numSubgroups; ++i)
    {
        testValue = WaveActiveSum(testValue);
    }

    outputBuffer[0] = testValue;
}
