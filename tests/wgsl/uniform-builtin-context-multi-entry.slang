//TEST:SIMPLE(filecheck=CHECK): -target wgsl -whole-program
//TEST:SIMPLE(filecheck=NOCTX): -target wgsl -whole-program

// When a module with several entry points is compiled into one output (`-whole-program`),
// `translateGlobalVaryingVar` links the lane-count shadow to the parameter of only the first entry
// point that reads it (shader-slang/slang#13687), so `mainB` never initializes it in either form.
// The shadow stays a `var<private>`, which leaves the module where it was before contexts: Tint
// rejects it whenever uniformity matters, instead of accepting a context that would silently carry a
// wrong value. These checks therefore pin output that is known to be wrong for `mainB`; a fix for
// shader-slang/slang#13687 is expected to change them.

// CHECK: var<private> [[V:[A-Za-z0-9_]+]] : u32;
// CHECK: fn laneCount{{[_0-9]*}}() -> u32
// CHECK: return [[V]];

// NOCTX-NOT: struct KernelContext

RWStructuredBuffer<uint> outputBuffer;

[noinline]
uint laneCount()
{
    return WaveGetLaneCount();
}

[shader("compute")]
[numthreads(64, 1, 1)]
void mainA(uint3 tid: SV_DispatchThreadID)
{
    outputBuffer[tid.x] = 256 / laneCount();
}

[shader("compute")]
[numthreads(64, 1, 1)]
void mainB(uint3 tid: SV_DispatchThreadID)
{
    outputBuffer[64 + tid.x] = 512 / laneCount();
}
