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
    for (;;)
    {
        if (i >= limits[tid.x])
            break;
        i++;
        if ((i & 1) == 0)
            continue;
        if (i == 7)
            break;
        sum += i;
    }
    results[tid.x] = sum + i;
}

// WGSL-NOT: for(;;)
// WGSL: loop
// WGSL: break;
// WGSL: continue;
// WGSL: break;

// SPIRV: OpEntryPoint GLCompute
