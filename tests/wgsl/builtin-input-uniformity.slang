//TEST:SIMPLE(filecheck=CHECK): -target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=TINT): -target wgsl-spirv-asm -entry computeMain -stage compute

// A built-in input that a function other than the entry point reads (here `subgroup_size`, through
// WaveGetLaneCount) is passed to that function as a parameter instead of through a module-scope
// `var<private>`. WGSL's uniformity analysis treats every read of a `var<private>` as possibly
// non-uniform, so a subgroup operation in a loop bounded by the subgroup size failed validation
// (#8774). Through a parameter the value stays uniform, and Tint accepts the module.

// CHECK-NOT: var<private>
// CHECK: fn helper_{{[0-9]+}}( v_{{[0-9]+}} : i32,  [[SIZE:[A-Za-z0-9_]+]] : u32) -> i32
// CHECK: u32(64) / [[SIZE]]
// CHECK: subgroupAdd(
// CHECK: fn computeMain({{.*}}@builtin(subgroup_size) [[LANES:[A-Za-z0-9_]+]] : u32)
// CHECK-NOT: var<private>
// CHECK: u32(64) / [[LANES]]
// CHECK: helper_{{[0-9]+}}({{.*}}, [[LANES]])

// TINT: OpGroupNonUniformIAdd

RWStructuredBuffer<int> outputBuffer;

int helper(int v)
{
    int r = v;
    for (uint i = 0; i < 64 / WaveGetLaneCount(); ++i)
        r = WaveActiveSum(r);
    return r;
}

[shader("compute")]
[numthreads(64, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int t = 1;
    for (uint i = 0; i < 64 / WaveGetLaneCount(); ++i)
        t = WaveActiveSum(t);
    outputBuffer[dispatchThreadID.x] = t + helper(int(dispatchThreadID.x));
}
