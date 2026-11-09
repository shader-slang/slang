// A function that is only called, not compiled as an entry point, gets no SPIR-V execution modes
// from its attributes. `helper` carries attributes that map to entry-point-scoped execution modes
// (`OutputVertices`, `Quads` and `MaximallyReconvergesKHR`); an `OpExecutionMode` that targets a
// function without an `OpEntryPoint` is invalid SPIR-V. The cell uses -O0 so `helper` survives as
// an `OpFunction`.

//TEST:SIMPLE(filecheck=CHECK): -target spirv -entry computeMain -stage compute -emit-spirv-directly -O0

RWStructuredBuffer<float> buf;

[domain("quad")]
[outputcontrolpoints(4)]
[MaximallyReconverges]
void helper(uint i)
{
    buf[i] = 1.0;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    helper(tid.x);
}

// CHECK: OpEntryPoint GLCompute %[[EP:[A-Za-z0-9_]+]]
// CHECK-NOT: OpExecutionMode {{%[A-Za-z0-9_]+}} {{OutputVertices|Quads|MaximallyReconvergesKHR}}
// CHECK: OpExecutionMode %[[EP]] LocalSize 1 1 1
// CHECK-NOT: OpExecutionMode {{%[A-Za-z0-9_]+}} {{OutputVertices|Quads|MaximallyReconvergesKHR}}
// CHECK: OpName %[[HELPER:[A-Za-z0-9_]+]] "helper"
// CHECK: %[[HELPER]] = OpFunction
