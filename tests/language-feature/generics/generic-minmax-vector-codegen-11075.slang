//TEST:SIMPLE(filecheck=CPP):-target cpp -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry computeMain -stage compute

// Regression test for shader-slang/slang#11075: on cpp/cuda a generic `IFloat` `min`/`max` on a
// vector lowers to per-element scalar calls, the same code the concrete vector overload emits.

T genMin<T : IFloat>(T a, T b)
{
    return min(a, b);
}
T genMax<T : IFloat>(T a, T b)
{
    return max(a, b);
}

RWStructuredBuffer<float3> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    float3 a = outputBuffer[0], b = outputBuffer[1];
    outputBuffer[2] = genMin(a, b);
    outputBuffer[3] = genMax(a, b);
}

// The concrete vector overload applies the scalar helper to each element, so we match the
// element accessor inside the call rather than the helper name alone (the helpers themselves are
// declared in the prelude).
// CPP-DAG: F32_min((_slang_vector_get_element(
// CPP-DAG: F32_max((_slang_vector_get_element(
// CUDA-DAG: F32_min((_slang_vector_get_element(
// CUDA-DAG: F32_max((_slang_vector_get_element(
