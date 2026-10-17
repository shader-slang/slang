//TEST:SIMPLE(filecheck=SAME): -target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=DISTINCT): -target wgsl -entry computeMainDistinct -stage compute

// Address-space specialization must reuse a specialization when a caller is processed again.
// `f`, `g` and `h` are called 3, 2 and 1 times, and each passes `Span.load` a pointer into the
// uniform address space: they must all share one `Span_load`, not get one copy each (#13643).

// SAME: fn Span_load_{{[0-9]+}}(
// SAME-NOT: fn Span_load_{{[0-9]+}}(
// SAME: fn f_{{[0-9]+}}(

// When the pointers really are in different address spaces (uniform vs. workgroup), the
// specializations differ and both must still be emitted.

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
