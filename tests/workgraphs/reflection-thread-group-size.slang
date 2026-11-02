// Work-graph node entry points report their threadgroup size in JSON reflection, like the other
// stages that take `[numthreads]`. A thread-launch node cannot declare `[numthreads]` and runs
// one thread per record, so it reports 1x1x1.

//TEST:REFLECTION(filecheck=BROADCAST): -target hlsl -stage node -entry broadcastNode -profile lib_6_8 -experimental-feature -no-codegen
//TEST:REFLECTION(filecheck=THREAD): -target hlsl -stage node -entry threadNode -profile lib_6_8 -experimental-feature -no-codegen

// BROADCAST: "stage": "node"
// BROADCAST: "threadGroupSize": [8, 4, 1]

// THREAD: "stage": "node"
// THREAD: "threadGroupSize": [1, 1, 1]

import experimental.workgraph;

struct Record
{
    uint3 grid : SV_DispatchGrid;
};

[shader("node")]
[NodeLaunch("broadcasting")]
[NodeMaxDispatchGrid(16, 16, 1)]
[numthreads(8, 4, 1)]
void broadcastNode(DispatchNodeInputRecord<Record> input, RWBuffer<uint> output)
{
    output[0] = input.Get().grid.x;
}

[shader("node")]
[NodeLaunch("thread")]
void threadNode(RWBuffer<uint> output)
{
    output[0] = 1;
}
