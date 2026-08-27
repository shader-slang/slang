//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=PTX): -target ptx -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CPP): -target cpp -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METAL): -target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=HLSL): -target hlsl -entry computeMain -stage compute

// Regression test for issue #12635: a nested global `static const` array must stay a single static
// initializer, not be split into per-row globals that the outer array references (which NVRTC rejects
// as dynamic initialization of a `__device__` variable). The fold is shared across the C-like targets,
// so this also exercises C++/Metal/HLSL. CUDA/C++ (`FixedArray<T,N>`) and Metal (`array<T,N>`) wrap
// arrays in a struct, so their initializers need an explicit struct brace at each level (`{ { ... } }`);
// HLSL uses native arrays and stays single-brace.

static const int values[2][2] = {
    { 1, 2 },
    { 3, 4 }
};

// depth-3 nesting: guards the recursive brace count.
static const int cube[2][2][2] = {
    { { 1, 2 }, { 3, 4 } },
    { { 5, 6 }, { 7, 8 } }
};

// The inner rows fold into the outer initializer (no separate per-row global emitted). The `-NOT`
// excludes the hoisted per-row global; the outer decls carry the extra dimension so they do not match
// the single-dimension `-NOT` pattern. `-DAG` because `values` and `cube` may emit in either order.
// CUDA-NOT: static const FixedArray<int, 2>{{ *}}_S
// CUDA-DAG: static const FixedArray<FixedArray<int, 2>{{.*}}values{{.*}}= { {
// CUDA-DAG: static const FixedArray<FixedArray<FixedArray<int, 2>{{.*}}cube{{.*}}= { {

// CPP-NOT: static const FixedArray<int, 2>{{ *}}_S
// CPP-DAG: static const FixedArray<FixedArray<int32_t, 2>{{.*}}values{{.*}}= { { { { int(1), int(2) } }, { { int(3), int(4) } } } }

// Metal's array<T,N> is a struct wrapper too, so the nested initializer must carry the struct brace.
// METAL-NOT: constant array<int, {{.*}}> _S
// METAL-DAG: array<array<int, int(2)>, int(2)>{{.*}}values{{.*}}= { { { { int(1), int(2) } }, { { int(3), int(4) } } } }

// HLSL uses native arrays: single brace per level, and the rows still fold inline (no per-row global).
// HLSL-NOT: static const int {{[a-zA-Z_]*}}_S{{[0-9]}}
// HLSL-DAG: static const int{{ +}}values{{[a-zA-Z0-9_]*}}[int(2)][int(2)] = { { int(1), int(2) }, { int(3), int(4) } }

RWStructuredBuffer<int> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    // Runtime index of the outer arrays — the construct that requires `values`/`cube` to be
    // declarations (so the folded, inlined initializers are what is emitted).
    // PTX: .visible .entry
    output[0] = values[tid.x][tid.y] + cube[tid.x][tid.y][tid.z];
}
