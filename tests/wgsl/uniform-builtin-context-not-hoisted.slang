//TEST:SIMPLE(filecheck=WAVEINDEX): -target wgsl -entry waveIndexMain -stage compute
//TEST:SIMPLE(filecheck=NOCTX): -target wgsl -entry waveIndexMain -stage compute
//TEST:SIMPLE(filecheck=FRAGMENT): -target wgsl -entry fragmentMain -stage fragment
//TEST:SIMPLE(filecheck=NOCTX): -target wgsl -entry fragmentMain -stage fragment
//TEST:SIMPLE(filecheck=WRITTEN): -target wgsl -entry writtenMain -stage compute
//TEST:SIMPLE(filecheck=NOCTX): -target wgsl -entry writtenMain -stage compute

// Built-in input shadows stay module-scope `var<private>` copies on WGSL unless the value is
// workgroup-uniform in a compute shader and is never stored to after its initialization:
// - `subgroup_id` is uniform only within a subgroup, so it cannot share a context with
//   workgroup-uniform values that a workgroup barrier depends on;
// - `subgroup_size` is not uniform outside compute shaders;
// - a user-declared global input that the shader writes to can hold a non-uniform value.

// No context is introduced, whether a struct or a by-value parameter.
// NOCTX-NOT: {{[Kk]}}ernelContext

// WAVEINDEX: var<private> [[V:[A-Za-z0-9_]+]] : u32;
// WAVEINDEX: fn waveIndexOf{{[_0-9]*}}() -> u32

// FRAGMENT: var<private> [[V:[A-Za-z0-9_]+]] : u32;
// FRAGMENT: fn laneCountOf{{[_0-9]*}}() -> u32

// WRITTEN: var<private> [[V:[A-Za-z0-9_]+]] : u32;
// WRITTEN: fn readWritten{{[_0-9]*}}() -> u32

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
