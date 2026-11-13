//TEST:SIMPLE(filecheck=WGSL): -target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=ONEPRIV): -target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METAL): -target metal -entry computeMain -stage compute
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-wgpu -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-vk -compute -shaderobj -output-using-type

// WGSL's uniformity analysis treats every read of a module-scope `var<private>` as possibly
// non-uniform, so a subgroup operation in a loop bounded by `WaveGetLaneCount()` fails validation
// when the lane count is read from the private shadow copy of `@builtin(subgroup_size)`
// (shader-slang/slang#8774). The lane count is uniform in a compute shader, and it is the only
// uniform built-in this shader reads, so its value is passed by value to the functions that need
// it. The lane index is not uniform and stays a `var<private>`.

// WGSL: enable subgroups;
// WGSL: var<private> [[LI:[A-Za-z0-9_]+]] : u32;
// WGSL: fn laneIsFirst{{[_0-9]*}}() -> u32
// WGSL: [[LI]] == u32(0)
// WGSL: fn numSubgroups{{[_0-9]*}}({{ *}}[[P:[A-Za-z0-9_]+]] : u32) -> u32
// WGSL: u32(256) / [[P]];
// WGSL: fn sumAll{{[_0-9]*}}({{ *}}[[Q:[A-Za-z0-9_]+]] : u32) -> u32
// WGSL: numSubgroups{{[_0-9]*}}([[Q]])
// WGSL: fn computeMain(
// WGSL-SAME: @builtin(subgroup_size) [[LC:[A-Za-z0-9_]+]] : u32
// WGSL: u32(256) / [[LC]];
// WGSL: u32(256) / [[LC]];
// WGSL: sumAll{{[_0-9]*}}([[LC]])

// Only the lane index keeps a module-scope copy.
// ONEPRIV: var<private>
// ONEPRIV-NOT: var<private>
// ONEPRIV-NOT: struct KernelContext

// Metal keeps moving every thread-local global into its `KernelContext`.
// METAL: laneIsFirst{{[_0-9]*}}(KernelContext{{[_0-9]*}} thread*
// METAL: sumAll{{[_0-9]*}}(KernelContext{{[_0-9]*}} thread*

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

static const uint kGroupSize = 256;

[noinline]
uint numSubgroups()
{
    return kGroupSize / WaveGetLaneCount();
}

// Reads the lane count only through a callee, so the value reaches it through two calls.
[noinline]
uint sumAll()
{
    uint total = 0;
    for (uint i = 0; i < numSubgroups(); ++i)
        total += WaveActiveSum(1u);
    return total;
}

[noinline]
uint laneIsFirst()
{
    return WaveGetLaneIndex() == 0 ? 1u : 0u;
}

[shader("compute")]
[numthreads(kGroupSize, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    // A read in a loop condition, after a wave operation, sits in a different block from the
    // entry-point initialization of the shadow.
    uint inMain = WaveActiveSum(0u);
    for (uint i = 0; i < kGroupSize / WaveGetLaneCount(); ++i)
        inMain += WaveActiveSum(1u);

    // A read after an `if` is likewise in a later block.
    if (tid.x == 0)
        outputBuffer[3] = 0;
    uint afterIf = 0;
    let n = kGroupSize / WaveGetLaneCount();
    for (uint i = 0; i < n; ++i)
        afterIf += WaveActiveSum(1u);

    let firsts = WaveActiveSum(laneIsFirst());
    let total = sumAll();
    if (tid.x == 0)
    {
        outputBuffer[0] = total;
        outputBuffer[1] = inMain;
        outputBuffer[2] = afterIf + firsts - 1;
    }
}

// BUF: 256
// BUF-NEXT: 256
// BUF-NEXT: 256
// BUF-NEXT: 0
