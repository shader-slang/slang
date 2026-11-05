// WaveGetNumWaves maps to WGSL's num_subgroups built-in (as WaveGetWaveIndex maps to subgroup_id)
// instead of being computed from the workgroup size, which an override-sized workgroup does not
// have as a constant.
//TEST:SIMPLE(filecheck=WGSL): -target wgsl -stage compute -entry computeMain
// No wgsl-spirv-asm run: the bundled slang-tint predates subgroup_id/num_subgroups (#13861).

RWStructuredBuffer<uint> outputBuffer;

// WGSL: enable subgroups;
// WGSL: fn computeMain({{.*}}@builtin(num_subgroups) [[COUNT:[A-Za-z0-9_]+]] : u32
// WGSL-SAME: @builtin(subgroup_id)
// WGSL: = [[COUNT]];
[numthreads(64, 2, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    outputBuffer[tid.x] = WaveGetNumWaves() * 1000 + WaveGetWaveIndex();
}
