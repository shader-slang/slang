// Metal declares `groupshared` memory as a `threadgroup` local of the entry point. A
// store that directly follows that declaration must stay a separate assignment rather
// than become an initializer, which Metal's `threadgroup` variables do not take.

//TEST:SIMPLE(filecheck=MESH): -target metal -stage mesh -entry meshMain
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -stage mesh -entry meshMain
//TEST:SIMPLE(filecheck=TASK): -target metal -stage amplification -entry taskMain
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -stage amplification -entry taskMain
//TEST:SIMPLE(filecheck=KERNEL): -target metal -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -stage compute -entry computeMain

// METALLIB-NOT: error :

// MESH-NOT: threadgroup float {{.*}} =
// MESH: threadgroup float [[S:meshShared_[0-9]+]];
// MESH: [[S]] = 5.0f;
// MESH: threadgroup_barrier
// MESH: float4([[S]],

struct Vertex
{
    float4 pos : SV_Position;
}

groupshared float meshShared;

[outputtopology("triangle")]
[numthreads(3, 1, 1)]
void meshMain(uint tid: SV_GroupThreadID, out vertices Vertex verts[3], out indices uint3 tris[1])
{
    meshShared = 5.0;
    GroupMemoryBarrierWithGroupSync();
    SetMeshOutputCounts(3, 1);
    Vertex v;
    v.pos = float4(meshShared, 0, 0, 1);
    verts[tid] = v;
    tris[0] = uint3(0, 1, 2);
}

// TASK-NOT: threadgroup uint {{.*}} =
// TASK: threadgroup uint [[S:taskShared_[0-9]+]];
// TASK: [[S]] = tid_{{[0-9]+}}.x;
// TASK: threadgroup_barrier
// TASK: = [[S]];

struct Payload
{
    uint count;
}

groupshared uint taskShared;

[numthreads(32, 1, 1)]
void taskMain(uint tid: SV_GroupThreadID)
{
    taskShared = tid;
    GroupMemoryBarrierWithGroupSync();
    Payload p;
    p.count = taskShared;
    DispatchMesh(1, 1, 1, p);
}

// The atomic keeps the variable alive, so the store is not removed as dead.
// KERNEL-NOT: threadgroup uint {{.*}} =
// KERNEL: threadgroup uint [[S:kernelShared_[0-9]+]];
// KERNEL: [[S]] = tid_{{[0-9]+}}.x;
// KERNEL: threadgroup_barrier
// KERNEL: atomic_fetch_add_explicit({{.*}}&[[S]]

groupshared uint kernelShared;

[numthreads(64, 1, 1)]
void computeMain(uint3 tid: SV_GroupThreadID)
{
    kernelShared = tid.x;
    GroupMemoryBarrierWithGroupSync();
    uint old;
    InterlockedAdd(kernelShared, 1, old);
}
