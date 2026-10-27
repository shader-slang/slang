// Test that `-inline-single-call-functions` inlines a chain of single-call
// wrappers into the entry point, and keeps functions that are called twice
// or marked `[noinline]`. Without the option, or in a whole-program (library)
// compile, every function stays.

//TEST:SIMPLE(filecheck=INLINE_HLSL): -target hlsl -entry computeMain -stage compute -inline-single-call-functions
//TEST:SIMPLE(filecheck=INLINE_WGSL): -target wgsl -entry computeMain -stage compute -inline-single-call-functions
//TEST:SIMPLE(filecheck=KEEP_HLSL): -target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=KEEP_WGSL): -target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=KEEP_HLSL): -target hlsl -profile lib_6_6 -whole-program -inline-single-call-functions

RWStructuredBuffer<float4> buffer;

// Called from two places, so it must stay a function.
float4 sharedHelper(float4 a, float4 b, uint k)
{
    if ((k & 3u) == 0u)
        return a * b + 0.5;
    return a.yzwx - b * 0.25;
}

// Called once, but the user asked for it to stay a function.
[noinline]
float4 keptNoInline(float4 a)
{
    return a * a + buffer[3];
}

float4 bigBody(uint t)
{
    float4 a = buffer[t];
    float4 b = buffer[t + 1u];
    if (a.x > 0.5)
        a = sharedHelper(a, b, t);
    else
        b = sharedHelper(b, a, t ^ 7u);
    return keptNoInline(a + b);
}

float4 wrapLevel0(uint t) { return bigBody(t) * 1.0001; }
float4 wrapLevel1(uint t) { return wrapLevel0(t) * 1.0002; }
float4 wrapLevel2(uint t) { return wrapLevel1(t) * 1.0003; }

[numthreads(64, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    buffer[tid.x] = wrapLevel2(tid.x);
}

// The kept functions are defined before the entry point in every lane.
// INLINE_HLSL: sharedHelper
// INLINE_HLSL: keptNoInline
// INLINE_HLSL-NOT: wrapLevel
// INLINE_HLSL-NOT: bigBody
// INLINE_HLSL: void computeMain(
// INLINE_HLSL-NOT: wrapLevel
// INLINE_HLSL-NOT: bigBody
// INLINE_HLSL: keptNoInline{{.*}}(

// INLINE_WGSL: fn sharedHelper
// INLINE_WGSL: fn keptNoInline
// INLINE_WGSL-NOT: wrapLevel
// INLINE_WGSL-NOT: bigBody
// INLINE_WGSL: fn computeMain(
// INLINE_WGSL-NOT: wrapLevel
// INLINE_WGSL-NOT: bigBody
// INLINE_WGSL: keptNoInline{{.*}}(

// KEEP_HLSL-DAG: wrapLevel0{{.*}}(
// KEEP_HLSL-DAG: wrapLevel1{{.*}}(
// KEEP_HLSL-DAG: wrapLevel2{{.*}}(
// KEEP_HLSL-DAG: bigBody{{.*}}(
// KEEP_HLSL: void computeMain(

// KEEP_WGSL-DAG: fn wrapLevel0
// KEEP_WGSL-DAG: fn wrapLevel1
// KEEP_WGSL-DAG: fn wrapLevel2
// KEEP_WGSL-DAG: fn bigBody
// KEEP_WGSL: fn computeMain(
