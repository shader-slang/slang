// The Metal DispatchMesh intrinsic names the entry point's `_slang_mesh_payload` and `_slang_mgp`
// parameters, so a call in a helper must be inlined into each entry point that reaches it, and
// each entry point declares the two parameters exactly once however many calls it contains.

//TEST:SIMPLE(filecheck=METAL): -target metal
//TEST:SIMPLE(filecheck=METALLIB): -target metallib

// METALLIB: result code = 0

struct Payload { uint n; }
groupshared Payload s_payload;

void taskImpl(uint n)
{
    s_payload.n = n;
    DispatchMesh(1, 1, 1, s_payload);
}

// A helper no entry point reaches is dropped rather than emitted with the call still in it.
void deadHelper(uint n)
{
    s_payload.n = n;
    DispatchMesh(1, 1, 1, s_payload);
}

// METAL-NOT: _slang_mesh_payload
// METAL-NOT: deadHelper
// METAL: void taskA(uint3 {{[^,]*}}, Payload_0 object_data* _slang_mesh_payload {{[^,]*}}, mesh_grid_properties _slang_mgp)
// METAL: *_slang_mesh_payload = {{.*}}_slang_mgp.set_threadgroups_per_grid(uint3(
[shader("amplification")]
[numthreads(1, 1, 1)]
void taskA(uint tid : SV_GroupIndex)
{
    taskImpl(tid);
}

// METAL: void taskB(uint3 {{[^,]*}}, Payload_0 object_data* _slang_mesh_payload {{[^,]*}}, mesh_grid_properties _slang_mgp)
// METAL: *_slang_mesh_payload = {{.*}}_slang_mgp.set_threadgroups_per_grid(uint3(
[shader("amplification")]
[numthreads(1, 1, 1)]
void taskB(uint tid : SV_GroupIndex)
{
    taskImpl(tid + 1u);
}

// METAL: void taskBranch(uint3 {{[^,]*}}, Payload_0 object_data* _slang_mesh_payload {{[^,]*}}, mesh_grid_properties _slang_mgp)
// METAL: *_slang_mesh_payload = {{.*}}_slang_mgp.set_threadgroups_per_grid(uint3(
// METAL: *_slang_mesh_payload = {{.*}}_slang_mgp.set_threadgroups_per_grid(uint3(
[shader("amplification")]
[numthreads(1, 1, 1)]
void taskBranch(uint tid : SV_GroupIndex)
{
    if (tid == 0u)
    {
        s_payload.n = 1u;
        DispatchMesh(1, 1, 1, s_payload);
    }
    else
    {
        s_payload.n = 2u;
        DispatchMesh(2, 1, 1, s_payload);
    }
}

void taskOuter(uint n)
{
    taskImpl(n * 2u);
}

// A call two helpers deep takes more than one inlining pass to reach the entry point.
// METAL: void taskNested(uint3 {{[^,]*}}, Payload_0 object_data* _slang_mesh_payload {{[^,]*}}, mesh_grid_properties _slang_mgp)
// METAL: *_slang_mesh_payload = {{.*}}_slang_mgp.set_threadgroups_per_grid(uint3(
[shader("amplification")]
[numthreads(1, 1, 1)]
void taskNested(uint tid : SV_GroupIndex)
{
    taskOuter(tid);
}

void taskImplGeneric<int N>(uint n)
{
    s_payload.n = n + uint(N);
    DispatchMesh(N, 1, 1, s_payload);
}

// A specialized generic helper is an ordinary function by the time the pass runs.
// METAL: void taskGeneric(uint3 {{[^,]*}}, Payload_0 object_data* _slang_mesh_payload {{[^,]*}}, mesh_grid_properties _slang_mgp)
// METAL: *_slang_mesh_payload = {{.*}}_slang_mgp.set_threadgroups_per_grid(uint3(
[shader("amplification")]
[numthreads(1, 1, 1)]
void taskGeneric(uint tid : SV_GroupIndex)
{
    taskImplGeneric<2>(tid);
}

// An entry point that never dispatches gets neither parameter.
// METAL: void taskNone(uint3 {{[^,)]*}})
// METAL-NOT: _slang_mesh_payload
[shader("amplification")]
[numthreads(1, 1, 1)]
void taskNone(uint tid : SV_GroupIndex)
{
    s_payload.n = tid;
}
