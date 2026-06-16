// builtin-volatile-raytmax-intersection.slang
//
// Regression test for shader-slang/slang#10528.
// `RayTmaxKHR` accessed via `RayTCurrent()` in an intersection shader
// must be decorated with `Volatile` under the GLSL450 memory model;
// the value reflects the current candidate hit and can change between
// accesses driven by traversal.

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry intersectionMain -stage intersection -emit-spirv-directly

RWStructuredBuffer<float> output;

// CHECK-DAG: OpDecorate %[[T:[A-Za-z0-9_]+]] BuiltIn RayTmaxKHR
// CHECK-DAG: OpDecorate %[[T]] Volatile
[shader("intersection")]
void intersectionMain()
{
    output[0] = RayTCurrent();
}
