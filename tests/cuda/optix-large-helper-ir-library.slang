//TEST:SIMPLE(filecheck=CHECK): -target cuda -O2 -validate-ir -line-directive-mode none

// An entryless library retains normal inlining and CUDA export linkage.
struct State { uint values[512]; };

uint libraryHelper(inout State state, uint index)
{
    state.values[index & 511] += index;
    return state.values[index & 511];
}

export uint libraryExport(inout State state, uint index)
{
    return libraryHelper(state, index);
}

// CHECK: static __device__ uint libraryHelper_
// CHECK: {{^}}__device__ uint libraryExport
// CHECK: libraryHelper_{{.*}}(
