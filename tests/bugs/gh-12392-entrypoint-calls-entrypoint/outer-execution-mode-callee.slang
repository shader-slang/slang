// An entry point can call imported entry points whose attributes map to SPIR-V execution modes
// (`[MaximallyReconverges]`, `[QuadDerivatives]`, `[RequireFullQuads]`). The callees are ordinary
// functions in this compile and have no `OpEntryPoint`, so none of their execution modes may be
// emitted; an `OpExecutionMode` on a non-entry function is invalid SPIR-V. The cells use -O0 so the
// callees survive as `OpFunction`s.

//TEST:SIMPLE(filecheck=CHECK-CS): -target spirv -entry computeMain -stage compute -emit-spirv-directly -O0
//TEST:SIMPLE(filecheck=CHECK-FS): -target spirv -entry fragmentMain -stage fragment -emit-spirv-directly -O0

import modes;

[shader("compute")]
[numthreads(32, 1, 1)]
void computeMain(uniform uint tid, uniform RWStructuredBuffer<float> buf)
{
    reconvergingKernel(tid, buf);
}

[shader("fragment")]
float4 fragmentMain(float4 c: COLOR) : SV_Target
{
    return quadDerivativesFragment(c) + fullQuadsFragment(c);
}

// CHECK-CS-NOT: {{maximal_reconvergence|MaximallyReconverges}}
// CHECK-CS: OpEntryPoint GLCompute
// CHECK-CS-NOT: {{maximal_reconvergence|MaximallyReconverges}}
// CHECK-CS: OpName %[[CALLEE:[A-Za-z0-9_]+]] "reconvergingKernel"
// CHECK-CS-NOT: {{maximal_reconvergence|MaximallyReconverges}}
// CHECK-CS: %[[CALLEE]] = OpFunction
// CHECK-CS-NOT: {{maximal_reconvergence|MaximallyReconverges}}

// CHECK-FS-NOT: {{quad_control|QuadControl|QuadDerivatives|RequireFullQuads}}
// CHECK-FS: OpEntryPoint Fragment
// CHECK-FS-NOT: {{quad_control|QuadControl|QuadDerivatives|RequireFullQuads}}
// CHECK-FS: OpName %[[QD:[A-Za-z0-9_]+]] "quadDerivativesFragment"
// CHECK-FS-NOT: {{quad_control|QuadControl|QuadDerivatives|RequireFullQuads}}
// CHECK-FS: %[[QD]] = OpFunction
// CHECK-FS-NOT: {{quad_control|QuadControl|QuadDerivatives|RequireFullQuads}}
