//TEST:SIMPLE(filecheck=CUDA): -target cuda -stage compute -entry computeMain -line-directive-mode none
//TEST:SIMPLE(filecheck=METAL): -target metal -stage compute -entry computeMain -line-directive-mode none

RWStructuredBuffer<uint> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 count : SV_WorkgroupCount)
{
    outputBuffer[0] = count.x + count.y + count.z;
}

// CUDA: gridDim
// METAL: threadgroups_per_grid
