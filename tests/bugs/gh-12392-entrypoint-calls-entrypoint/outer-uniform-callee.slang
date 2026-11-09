// An entry point (`outer`) can call an imported function (`inner`, from `callee`) that is itself
// declared as a compute entry point and takes `uniform` resource parameters and a system-value
// input. `inner` is compiled as an ordinary callee, so none of its parameters are given an
// entry-point parameter layout.

//TEST:SIMPLE(filecheck=CHECK-HLSL): -target hlsl -entry outer -stage compute
//TEST:SIMPLE(filecheck=CHECK-CUDA): -target cuda -entry outer -stage compute
//TEST:SIMPLE(filecheck=CHECK-WGSL): -target wgsl -entry outer -stage compute
//TEST:SIMPLE(filecheck=CHECK-METAL): -target metal -entry outer -stage compute
//TEST:SIMPLE(filecheck=CHECK-SPV): -target spirv -entry outer -stage compute -emit-spirv-directly -O0

import callee;

[shader("compute")]
[numthreads(64, 1, 1)]
void outer(uniform RWStructuredBuffer<uint> b, uniform uint n, uint3 tid: SV_DispatchThreadID)
{
    inner(b, n, tid);
}

// CHECK-HLSL-NOT: [numthreads
// CHECK-HLSL: {{^}}void [[INNER:inner_[0-9]+]](
// CHECK-HLSL: [numthreads(64, 1, 1)]
// CHECK-HLSL-NEXT: void outer(
// CHECK-HLSL: [[INNER]](

// CHECK-CUDA-NOT: __global__ {{.*}}inner
// CHECK-CUDA: __device__ void [[INNER:inner_[0-9]+]](
// CHECK-CUDA: extern "C" __global__ void outer(
// CHECK-CUDA: [[INNER]](

// CHECK-WGSL-NOT: @compute
// CHECK-WGSL: fn [[INNER:inner_[0-9]+]](
// CHECK-WGSL: @compute
// CHECK-WGSL-NEXT: @workgroup_size(64, 1, 1)
// CHECK-WGSL-NEXT: fn outer(
// CHECK-WGSL: [[INNER]](

// CHECK-METAL-NOT: {{\[\[kernel\]\]}}
// CHECK-METAL: {{^}}void [[INNER:inner_[0-9]+]](
// CHECK-METAL: {{\[\[kernel\]\]}} void outer(
// CHECK-METAL: [[INNER]](

// The SPIR-V cell uses -O0 so `inner` survives as an ordinary `OpFunction`; only the entry point
// gets the `LocalSize` execution mode.
// CHECK-SPV: OpEntryPoint GLCompute %[[EP:[A-Za-z0-9_]+]] "main"
// CHECK-SPV-NOT: OpExecutionMode {{%[A-Za-z0-9_]+}} LocalSize
// CHECK-SPV: OpExecutionMode %[[EP]] LocalSize 64 1 1
// CHECK-SPV-NOT: OpExecutionMode {{%[A-Za-z0-9_]+}} LocalSize
// CHECK-SPV: OpName %[[CALLEE:[A-Za-z0-9_]+]] "inner"
// CHECK-SPV: %[[CALLEE]] = OpFunction
