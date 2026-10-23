//TEST:SIMPLE(filecheck=WAVEINDEX): -target wgsl -entry waveIndexMain -stage compute
//TEST:SIMPLE(filecheck=NOCTX): -target wgsl -entry waveIndexMain -stage compute
//TEST:SIMPLE(filecheck=FRAGMENT): -target wgsl -entry fragmentMain -stage fragment
//TEST:SIMPLE(filecheck=NOCTX): -target wgsl -entry fragmentMain -stage fragment
//TEST:SIMPLE(filecheck=WRITTEN): -target wgsl -entry writtenMain -stage compute
//TEST:SIMPLE(filecheck=NOCTX): -target wgsl -entry writtenMain -stage compute
//TEST:SIMPLE(filecheck=CALLEEWRITE): -target wgsl -entry calleeWriteMain -stage compute
//TEST:SIMPLE(filecheck=NOCTX): -target wgsl -entry calleeWriteMain -stage compute
//TEST:SIMPLE(filecheck=INOUT): -target wgsl -entry inoutMain -stage compute
//TEST:SIMPLE(filecheck=NOCTX): -target wgsl -entry inoutMain -stage compute

// Built-in input shadows stay module-scope `var<private>` copies on WGSL unless the value is
// workgroup-uniform in a compute shader and is never stored to after its initialization:
// - `subgroup_id` is uniform only within a subgroup, so it cannot share a context with
//   workgroup-uniform values that a workgroup barrier depends on;
// - `subgroup_size` is not uniform outside compute shaders;
// - a user-declared global input that the shader writes to can hold a non-uniform value, whether
//   the entry point writes it, a callee writes one of its components, or it is passed `inout`.

// No context is introduced, whether a struct or a by-value parameter.
// NOCTX-NOT: {{[Kk]}}ernelContext

// WAVEINDEX: var<private> [[V:[A-Za-z0-9_]+]] : u32;
// WAVEINDEX: fn waveIndexOf{{[_0-9]*}}() -> u32

// FRAGMENT: var<private> [[V:[A-Za-z0-9_]+]] : u32;
// FRAGMENT: fn laneCountOf{{[_0-9]*}}() -> u32

// WRITTEN: var<private> [[V:[A-Za-z0-9_]+]] : u32;
// WRITTEN: fn readWritten{{[_0-9]*}}() -> u32

// CALLEEWRITE: var<private> [[GW:[A-Za-z0-9_]+]] : vec3<u32>;
// CALLEEWRITE: fn readCalleeWritten{{[_0-9]*}}() -> u32
// CALLEEWRITE: [[GW]].y

// INOUT: var<private> [[GI:[A-Za-z0-9_]+]] : vec3<u32>;
// INOUT: fn readInout{{[_0-9]*}}() -> u32
// INOUT: [[GI]].x

RWStructuredBuffer<uint> outputBuffer;

[noinline]
uint waveIndexOf()
{
    return WaveGetWaveIndex();
}

[shader("compute")]
[numthreads(64, 1, 1)]
void waveIndexMain(uint3 tid: SV_DispatchThreadID)
{
    outputBuffer[tid.x] = waveIndexOf();
}

[noinline]
uint laneCountOf()
{
    return WaveGetLaneCount();
}

[shader("fragment")]
float4 fragmentMain() : SV_Target
{
    return float4(float(laneCountOf()), 0, 0, 1);
}

in uint gWrittenLaneCount : SV_WaveLaneCount;

[noinline]
uint readWritten()
{
    return gWrittenLaneCount;
}

[shader("compute")]
[numthreads(64, 1, 1)]
void writtenMain(uint3 tid: SV_DispatchThreadID)
{
    if (tid.x == 0)
        gWrittenLaneCount = 3;
    outputBuffer[tid.x] = readWritten();
}

in uint3 gCalleeWrittenGroupId : SV_GroupID;

[noinline]
void clobberY(uint v)
{
    gCalleeWrittenGroupId.y = v;
}

[noinline]
uint readCalleeWritten()
{
    return gCalleeWrittenGroupId.y;
}

[shader("compute")]
[numthreads(64, 1, 1)]
void calleeWriteMain(uint3 tid: SV_DispatchThreadID)
{
    clobberY(tid.x);
    outputBuffer[tid.x] = readCalleeWritten();
}

in uint3 gInoutGroupId : SV_GroupID;

[noinline]
void bump(inout uint3 v)
{
    v.x += 1;
}

[noinline]
uint readInout()
{
    return gInoutGroupId.x;
}

[shader("compute")]
[numthreads(64, 1, 1)]
void inoutMain(uint3 tid: SV_DispatchThreadID)
{
    bump(gInoutGroupId);
    outputBuffer[tid.x] = readInout();
}
