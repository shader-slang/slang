// A generic entry point under the module-scope `layout(derivative_group_quadsNV) in;` qualifier
// keeps generic parameters as its `[numthreads]` operands until it is specialized. Module lowering
// therefore cannot check its thread counts against the quad requirement, and the check runs when
// the specialized entry point is emitted: a 4x4 tile compiles, and a 3x3 tile reports E31210.

//TEST:SIMPLE(filecheck=CHECK): -allow-glsl -target spirv -entry computeMain -specialize 4 -stage compute
//TEST:SIMPLE_EX(filecheck=ODD,exit-code=-1): -allow-glsl tests/glsl-intrinsic/compute-derivative/derivative-mode-generic-entry-point.slang -target spirv -entry computeMain -specialize 3 -stage compute

layout(derivative_group_quadsNV) in;

RWStructuredBuffer<float> buf;

[numthreads(tileSize, tileSize, 1)]
void computeMain<int8_t tileSize>(int2 id: SV_GroupThreadID)
{
    buf[id.x] = ddx(float(id.x));
}

// CHECK-NOT: E31210
// CHECK: OpEntryPoint GLCompute %[[EP:[A-Za-z0-9_]+]]
// CHECK-DAG: OpExecutionMode %[[EP]] DerivativeGroupQuadsKHR
// CHECK-DAG: OpExecutionMode %[[EP]] LocalSize 4 4 1

// ODD: error[E31210]
