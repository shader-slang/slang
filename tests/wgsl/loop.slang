//TEST:SIMPLE(filecheck=WGSL): -target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=SPIRV): -target wgsl-spirv-asm -entry computeMain -stage compute

// Verify that structured loops use WGSL's native `loop` statement while preserving conditional
// exits, explicit breaks, and continues.

[[vk::binding(0, 0)]]
StructuredBuffer<uint> limits;

[[vk::binding(1, 0)]]
RWStructuredBuffer<uint> results;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint i = 0;
    uint sum = 0;
    for (;; i++)
    {
        if (i >= limits[tid.x])
            break;
        if ((i & 1) == 0)
            continue;
        if (i == 7)
            break;
        sum += i;
    }
    results[tid.x] = sum + i;
}

// WGSL: var [[I:i_[0-9]+]] : u32 = u32(0);
// WGSL: loop
// WGSL: break;
// WGSL: [[I]] = [[I]] + u32(1);
// WGSL-NEXT: continue;
// WGSL: break;

// SPIRV: OpEntryPoint GLCompute
