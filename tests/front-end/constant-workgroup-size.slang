//TEST:SIMPLE(filecheck=CHECK): -target spirv -entry computeMain -stage compute

// This test checks that we can define a generic compute shader entrypoint
// where the workgroup size is determined by a generic parameter.

// CHECK: OpExecutionMode %computeMain LocalSize 4 4 1

// A link-time constant that isn't `int`
extern static const uint8_t tileSize = 4;

RWStructuredBuffer<int> outputBuffer;

// produces `IRNumThreadsDecoration(IntCast(tileSize), IntCast(tileSize), 1)`
// BUG-FIX to test: the IntCast inst should be inserted at the global scope, instead of at the end of
// some block of the function body.

[numthreads(tileSize,tileSize,1)]
void computeMain(int2 id : SV_GroupThreadID)
{
    static groupshared int data[tileSize*tileSize];
    data[id.y*tileSize + id.x] = id.x*id.y;
    
    GroupMemoryBarrierWithGroupSync();

    outputBuffer[id.y*tileSize + id.x] = data[(tileSize - id.y - 1)*tileSize + (tileSize - id.x - 1)];
}