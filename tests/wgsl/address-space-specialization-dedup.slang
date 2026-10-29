//TEST:SIMPLE(filecheck=SAME): -target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=DISTINCT): -target wgsl -entry computeMainDistinct -stage compute
//TEST:SIMPLE(filecheck=CLONED): -target wgsl -entry computeMainCloned -stage compute
//TEST:SIMPLE(filecheck=CLONED-LOAD): -target wgsl -entry computeMainCloned -stage compute

// Address-space specialization must reuse a specialization when a caller is processed again
// (#13643). `f`, `g` and `h` each pass `Span.load` a pointer into the uniform address space, so
// they must all share one `Span_load`. Each call site without pointer arguments puts its callee on
// the work list again, so `f` is processed three times, `g` twice and `h` once, and every later
// visit must key its already-rewritten call on `Span.load` and reuse the existing `Span_load`.
// The call counts differ on purpose: an unwanted copy made on a later visit is only emitted if
// some caller still calls it after the pass, and with unequal counts `f`, `g` and `h` would each
// end up calling a different copy.

// SAME-COUNT-1: fn Span_load_{{[0-9]+}}(
// SAME-NOT: fn Span_load_{{[0-9]+}}(

// When the pointers really are in different address spaces (uniform vs. workgroup), the
// specializations differ and both must be emitted. This run guards against over-merging (e.g. a
// key that ignores address spaces).

// DISTINCT-COUNT-2: fn Span_load_{{[0-9]+}}(
// DISTINCT-NOT: fn Span_load_{{[0-9]+}}(

// A caller that takes a pointer is itself specialized: `accumulate` is cloned for the
// function-space `acc`. `viaLocal` is processed twice, and on its second visit its call already
// targets that clone, which must be reused rather than cloned again. The clone's `Span_load`
// call must reuse the one `Span_load` that `f` also calls.

// CLONED-COUNT-1: fn accumulate_{{[0-9]+}}(
// CLONED-NOT: fn accumulate_{{[0-9]+}}(

// CLONED-LOAD-COUNT-1: fn Span_load_{{[0-9]+}}(
// CLONED-LOAD-NOT: fn Span_load_{{[0-9]+}}(

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

float4 accumulate(inout float4 acc, uint i)
{
    acc += params.data.load(i);
    return acc;
}

float4 viaLocal(uint i)
{
    float4 local = 0;
    return accumulate(local, i);
}

[shader("compute")]
[numthreads(64, 1, 1)]
void computeMainCloned(uint3 tid: SV_DispatchThreadID)
{
    float4 local = 0;
    outputBuffer[tid.x] = accumulate(local, tid.x) + viaLocal(tid.y) + viaLocal(tid.z) + f(tid.x);
}
