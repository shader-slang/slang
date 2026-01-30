//TEST:SIMPLE(filecheck=CHECK): -target spirv -entry computeMain -specialize 4 -stage compute

// This test checks that we can define a generic compute shader entrypoint
// where the workgroup size is determined by a generic parameter.

// CHECK: OpExecutionMode %computeMain LocalSize 4 4 1

RWStructuredBuffer<int> outputBuffer;

[numthreads(tileSize,tileSize,1)]
void computeMain<int8_t tileSize>(int2 id : SV_GroupThreadID)
{
    static groupshared int data[tileSize*tileSize];
    data[id.y*tileSize + id.x] = id.x*id.y;
    
    GroupMemoryBarrierWithGroupSync();

    outputBuffer[id.y*tileSize + id.x] = data[(tileSize - id.y - 1)*tileSize + (tileSize - id.x - 1)];
}