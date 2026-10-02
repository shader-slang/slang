//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry computeMain -stage compute -O3 -validate-ir -line-directive-mode none

struct State
{
    uint a;
    uint unused[1024];
    uint b;
};
struct Plain
{
    uint a;
    uint unused;
    uint b;
};

struct VectorState
{
    uint2 pair;
    uint unused[16];
};

struct ManyFields
{
    uint a;
    uint b;
    uint c;
    uint d;
    uint e;
    uint unused[16];
};

[noinline]
uint manyFields(ManyFields snapshot)
{
    return snapshot.a + snapshot.b + snapshot.c + snapshot.d + snapshot.e;
}

[noinline]
uint fourFields(ManyFields snapshot)
{
    return snapshot.a + snapshot.b + snapshot.c + snapshot.d;
}

// Source export is a downstream ABI boundary, distinct from IR linkage names.
export uint ordinaryExport(State snapshot)
{
    return snapshot.a + snapshot.b;
}

[CudaDeviceExport]
uint deviceExport(State snapshot)
{
    return snapshot.a + snapshot.b;
}

// Keep explicit linkage/name metadata and its aggregate calling convention.
[noinline]
__extern_cpp uint namedState(State snapshot)
{
    return snapshot.a + snapshot.b;
}

[noinline]
uint vectorState(VectorState snapshot)
{
    return snapshot.pair.x + snapshot.pair.y;
}

[noinline]
uint mutableState(inout State snapshot)
{
    snapshot.a += 7;
    return snapshot.a + snapshot.b;
}

[noinline]
uint borrowedState(__constref State snapshot)
{
    return snapshot.a + snapshot.b;
}

[noinline]
uint usesArray(State snapshot, uint index)
{
    return snapshot.a + snapshot.unused[index & 1023] + snapshot.b;
}

[noinline]
uint smallState(Plain snapshot)
{
    return snapshot.a + snapshot.b;
}

// Whole-value uses through an exported callee cannot be replaced with field reads.
[noinline]
uint forwardsWholeValue(State snapshot)
{
    return deviceExport(snapshot);
}

// CUDA-DAG: __device__ uint ordinaryExport_{{[0-9]+}}(State_{{[0-9]+}} *
// CUDA-DAG: __device__ uint deviceExport(State_{{[0-9]+}}
// CUDA-DAG: namedState(State_{{[0-9]+}} *
// CUDA-DAG: static __device__ __noinline__ uint vectorState_{{[0-9]+}}(ValueFields{{[a-zA-Z0-9_]*}} *
// CUDA-DAG: static __device__ __noinline__ uint manyFields_{{[0-9]+}}(ValueFields{{[a-zA-Z0-9_]*}} *
// CUDA-DAG: static __device__ __noinline__ uint fourFields_{{[0-9]+}}(ValueFields{{[a-zA-Z0-9_]*}} *
// CUDA-DAG: static __device__ __noinline__ uint mutableState_{{[0-9]+}}(State_{{[0-9]+}} *
// CUDA-DAG: static __device__ __noinline__ uint borrowedState_{{[0-9]+}}(State_{{[0-9]+}} *
// CUDA-DAG: static __device__ __noinline__ uint usesArray_{{[0-9]+}}(State_{{[0-9]+}} *
// CUDA-DAG: static __device__ __noinline__ uint smallState_{{[0-9]+}}(Plain_{{[0-9]+}} *
// CUDA-DAG: static __device__ __noinline__ uint forwardsWholeValue_{{[0-9]+}}(State_{{[0-9]+}} *

RWStructuredBuffer<uint> output;
[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    State state = {};
    state.a = tid.x;
    state.b = output[tid.x];
    Plain plain = { state.a, 0, state.b };
    VectorState vector = {};
    vector.pair = uint2(state.a, state.b);
    ManyFields many = {};
    many.a = state.a;
    many.b = state.b;
    many.c = tid.x;
    many.d = state.a;
    many.e = state.b;
    output[tid.x] = mutableState(state) + borrowedState(state) + usesArray(state, tid.x) +
                    smallState(plain) + forwardsWholeValue(state) + namedState(state) +
                    vectorState(vector) + manyFields(many) + fourFields(many);
}
