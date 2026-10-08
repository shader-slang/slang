// The HLSL-style atomic functions compile inside a function that is
// differentiated with both fwd_diff and bwd_diff. They cover a buffer
// element, `groupshared` memory, and a texel, an atomic whose original value
// feeds the differentiable result, and an atomic inside a loop.

//TEST:SIMPLE(filecheck=SPIRV): -target spirv -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=HLSL): -target hlsl -stage compute -entry computeMain

RWStructuredBuffer<float> floatBuffer;
RWStructuredBuffer<uint> counters;
RWTexture2D<uint> counterTexture;
groupshared uint groupCounter;

[Differentiable]
float f(float x, uint n)
{
    InterlockedAdd(counters[0], 1u);
    InterlockedAdd(groupCounter, 1u);
    InterlockedAdd(counterTexture[uint2(0, 0)], 1u);

    uint previous;
    InterlockedCompareExchange(counters[1], 0u, 5u, previous);

    uint original;
    InterlockedAdd(counters[2], 2u, original);
    float y = x * float(original + 1u);

    [MaxIters(8)]
    for (uint i = 0; i < n; i++)
    {
        InterlockedMax(counters[3], i);
        y = y * x;
    }
    return y;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    let forward = fwd_diff(f)(diffPair(floatBuffer[0], 1.0f), 3u);
    var backward = diffPair(floatBuffer[1], 0.0f);
    bwd_diff(f)(backward, 3u, 1.0f);
    floatBuffer[2] = forward.d + backward.d;
}

// SPIRV-DAG: OpAtomicIAdd
// SPIRV-DAG: OpAtomicCompareExchange
// SPIRV-DAG: OpAtomicUMax

// The patterns use generated names (`counters_0`, the derivative functions) so
// that they cannot match source lines echoed in a diagnostic.
// HLSL-DAG: void s_bwdProp_f_0(
// HLSL-DAG: DiffPair_float_0 s_fwd_f_0(
// HLSL-DAG: InterlockedAdd(counters_0[
// HLSL-DAG: InterlockedCompareExchange(counters_0[
// HLSL-DAG: InterlockedMax(counters_0[
