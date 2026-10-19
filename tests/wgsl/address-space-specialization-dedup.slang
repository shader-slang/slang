//TEST:SIMPLE(filecheck=SAME): -target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=DISTINCT): -target wgsl -entry computeMainDistinct -stage compute

// Address-space specialization must reuse a specialization when a caller is processed again.
// `f`, `g` and `h` each pass `Span.load` a pointer into the uniform address space, so they must all
// share one `Span_load`, not get one copy each (#13643). The call counts (3, 2 and 1) matter: each
// call site without pointer arguments puts its callee on the work list again, and before the fix
// each later visit specialized the specialization the call already targeted, giving f -> 3rd copy,
// g -> 2nd copy, h -> 1st copy. With equal counts the extra copies lose all their callers and are
// removed, which would hide the bug.

// SAME-COUNT-1: fn Span_load_{{[0-9]+}}(
// SAME-NOT: fn Span_load_{{[0-9]+}}(

// When the pointers really are in different address spaces (uniform vs. workgroup), the
// specializations differ and both must still be emitted. This run gives two copies with or
// without the fix: it guards against over-merging (e.g. a key that ignores address spaces).

// DISTINCT-COUNT-2: fn Span_load_{{[0-9]+}}(
// DISTINCT-NOT: fn Span_load_{{[0-9]+}}(

ByteAddressBuffer gPool;
RWStructuredBuffer<float4> outputBuffer;

struct Span
{
    uint offset;
    float4 load(uint i) { return gPool.Load<float4>(offset + i * 16); }
}

struct Params
{
    Span data;
};
uniform Params params;

groupshared Span gShared;

float4 f(uint i) { return params.data.load(i); }
float4 g(uint i) { return params.data.load(i + 1); }
float4 h(uint i) { return params.data.load(i + 2); }
float4 fromShared(uint i) { return gShared.load(i); }

[shader("compute")]
[numthreads(64, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    outputBuffer[tid.x] = f(tid.x) + f(tid.y) + f(tid.z) + g(tid.x) + g(tid.y) + h(tid.x);
}

[shader("compute")]
[numthreads(64, 1, 1)]
void computeMainDistinct(uint3 tid: SV_DispatchThreadID)
{
    if (tid.x == 0)
        gShared.offset = tid.y;
    GroupMemoryBarrierWithGroupSync();
    outputBuffer[tid.x] = f(tid.x) + f(tid.y) + fromShared(tid.x) + fromShared(tid.y);
}
