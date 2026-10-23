//TEST:SIMPLE(filecheck=CHECK): -target wgsl -whole-program
//TEST:SIMPLE(filecheck=NOCTX): -target wgsl -whole-program

// When a module with several entry points is compiled into one output (`-whole-program`),
// `translateGlobalVaryingVar` links the lane-count shadow to the parameter of only the first entry
// point that reads it (shader-slang/slang#13687). If the shadow were moved into a context, `mainB`
// would pass an uninitialized value to `laneCount`, so the shadow stays a `var<private>` until
// every entry point that reaches a read is linked.

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
