//TEST:SIMPLE(filecheck=CHECK):-target cpp -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK):-target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=NODYN):-target cpp -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=NODYN):-target cuda -entry computeMain -stage compute

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

// The vector map macros are fully unrolled, so the scalar helper is called once per element with
// direct component access (`a.x`, `a.y`, `a.z`). We match the component accessors inside the call
// rather than the helper name alone, because the helpers themselves are declared in the prelude.
// CHECK-DAG: F32_min(({{.*}}.x), ({{.*}}.x))
// CHECK-DAG: F32_min(({{.*}}.y), ({{.*}}.y))
// CHECK-DAG: F32_min(({{.*}}.z), ({{.*}}.z))
// CHECK-DAG: F32_max(({{.*}}.x), ({{.*}}.x))
// CHECK-DAG: F32_max(({{.*}}.y), ({{.*}}.y))
// CHECK-DAG: F32_max(({{.*}}.z), ({{.*}}.z))

// No element is read through a dynamic index, and no vector element is written in a loop.
// NODYN-NOT: _slang_vector_get_element
// NODYN-NOT: {{result_[0-9]+}}[
