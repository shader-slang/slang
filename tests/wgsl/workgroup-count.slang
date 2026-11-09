// WorkgroupCount() maps to WGSL's num_workgroups built-in.
//TEST:SIMPLE(filecheck=WGSL): -target wgsl -stage compute -entry computeMain
// Validate the emitted WGSL through Tint (ignored where the downstream compiler is absent).
//TEST:SIMPLE(filecheck=SPV): -target wgsl-spirv-asm -stage compute -entry computeMain

RWStructuredBuffer<uint> outputBuffer;

// SPV: OpEntryPoint
// SPV: BuiltIn NumWorkgroups
// WGSL: fn computeMain({{.*}}@builtin(num_workgroups) [[COUNT:[A-Za-z0-9_]+]] : vec3<u32>
// WGSL: [[COUNT]]
[numthreads(8, 8, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    uint3 groups = WorkgroupCount();
    uint width = groups.x * 8;
    outputBuffer[tid.y * width + tid.x] = groups.x * groups.y * groups.z;
}
