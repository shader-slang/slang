//TEST:SIMPLE(filecheck=INLINE): -target cuda -entry missMain -stage miss -O1 -validate-ir -line-directive-mode none
//TEST:SIMPLE(filecheck=INLINE): -target cuda -entry missMain -stage miss -O2 -validate-ir -line-directive-mode none
//TEST:SIMPLE(filecheck=KEEP): -target cuda -entry missMain -stage miss -O0 -validate-ir -line-directive-mode none
//TEST:SIMPLE(filecheck=INLINE): -target cuda -entry closestHitMain -stage closesthit -O2 -validate-ir -line-directive-mode none
//TEST:SIMPLE(filecheck=INLINE): -target cuda -entry anyHitMain -stage anyhit -O2 -validate-ir -line-directive-mode none
//TEST:SIMPLE(filecheck=INLINE): -target cuda -entry intersectionMain -stage intersection -O2 -validate-ir -line-directive-mode none
//TEST:SIMPLE(filecheck=INLINE): -target cuda -entry missMain -entry closestHitMain -O2 -validate-ir -line-directive-mode none
//TEST:SIMPLE(filecheck=KEEP): -target cuda -entry rayGen -stage raygeneration -O2 -validate-ir -line-directive-mode none
//TEST:SIMPLE(filecheck=KEEP): -target cuda -entry computeMain -stage compute -O2 -validate-ir -line-directive-mode none
//TEST:SIMPLE(filecheck=KEEP): -target cuda -entry callableMain -stage callable -O2 -validate-ir -line-directive-mode none
//TEST:SIMPLE(filecheck=KEEP): -target cuda -entry rayGen -entry missMain -O2 -validate-ir -line-directive-mode none
//TEST:SIMPLE(filecheck=INLINE): -target cuda -entry missMain -stage miss -O3 -validate-ir -line-directive-mode none

struct LargeState { uint values[512]; };
struct BelowThresholdState { uint values[511]; };
struct Payload { uint value; };
RWStructuredBuffer<uint> values;

uint largeStateHelper(inout LargeState state, uint index)
{
    state.values[index & 511] += index;
    return state.values[index & 511];
}
uint belowThresholdHelper(inout BelowThresholdState state, uint index)
{
    state.values[index % 511] += index;
    return state.values[index % 511];
}
[noinline]
uint retainedLargeCall(inout LargeState state, uint index)
{
    state.values[index & 511] += 2;
    return state.values[index & 511];
}
[CudaDeviceExport]
uint exportedLargeCall(inout LargeState state, uint index)
{
    state.values[index & 511] += 3;
    return state.values[index & 511];
}
export uint sourceExport(inout LargeState state, uint index)
{
    state.values[index & 511] += 4;
    return state.values[index & 511];
}

// The exact 2 KiB helper disappears; below-threshold and explicit exclusions
// retain their definitions and calls. Source exports keep their external definition
// even when local calls are inlined. CUDA device exports keep their calls too.
// INLINE-NOT: largeStateHelper_
// INLINE-DAG: static __device__ uint belowThresholdHelper_
// INLINE-DAG: static __device__ __noinline__ uint retainedLargeCall_
// INLINE-DAG: {{^}}__device__ uint exportedLargeCall
// INLINE-DAG: {{^}}__device__ uint sourceExport
// INLINE-NOT: largeStateHelper_
// INLINE: extern "C" __global__ void
// INLINE-DAG: belowThresholdHelper_{{.*}}(
// INLINE-DAG: retainedLargeCall_{{.*}}(
// INLINE-DAG: exportedLargeCall(
// INLINE-NOT: sourceExport
// INLINE-NOT: largeStateHelper_
// KEEP: static __device__ uint largeStateHelper_

[ForceInline]
uint exercise(uint index)
{
    LargeState state = {};
    BelowThresholdState small = {};
    return largeStateHelper(state, index) + belowThresholdHelper(small, index) +
        retainedLargeCall(state, index) + exportedLargeCall(state, index) + sourceExport(state, index);
}

[shader("raygeneration")]
void rayGen()
{
    uint index = DispatchRaysIndex().x;
    values[index] = exercise(index);
}
[shader("miss")]
void missMain(inout Payload payload)
{
    payload.value = exercise(payload.value);
}
[shader("closesthit")]
void closestHitMain(inout Payload payload, in BuiltInTriangleIntersectionAttributes attributes)
{
    payload.value = exercise(payload.value + uint(attributes.barycentrics.x));
}
[shader("anyhit")]
void anyHitMain(inout Payload payload, in BuiltInTriangleIntersectionAttributes attributes)
{
    payload.value = exercise(payload.value + uint(attributes.barycentrics.y));
}
[shader("intersection")]
void intersectionMain()
{
    values[0] = exercise(PrimitiveIndex());
}
[shader("callable")]
void callableMain(inout Payload payload)
{
    payload.value = exercise(payload.value);
}
[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 index : SV_DispatchThreadID)
{
    values[index.x] = exercise(index.x);
}
