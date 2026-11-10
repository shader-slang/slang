// An entry point (`compute_main`) can call a function that is itself declared as an entry point
// (`k`, imported from `inner` and marked `[shader("compute")]`). Only the entry point selected for
// this compile is compiled as one; `k` is compiled as an ordinary callee on every target.

//TEST:SIMPLE(filecheck=CHECK-HLSL): -target hlsl -entry compute_main -stage compute
//TEST:SIMPLE(filecheck=CHECK-CUDA): -target cuda -entry compute_main -stage compute
//TEST:SIMPLE(filecheck=CHECK-WGSL): -target wgsl -entry compute_main -stage compute
//TEST:SIMPLE(filecheck=CHECK-METAL): -target metal -entry compute_main -stage compute
// The SPIR-V cell uses -O0 so `k` is not inlined away and survives as an ordinary callee. A called
// `[numthreads]` function must not get an entry-point-scoped `OpExecutionMode ... LocalSize`; only
// the selected entry point may.
//TEST:SIMPLE(filecheck=CHECK-SPV): -target spirv -entry compute_main -stage compute -emit-spirv-directly -O0

import inner;

[shader("compute")]
[numthreads(32, 1, 1)]
void compute_main(
    int3 physical_thread_id: SV_DispatchThreadID,
    uniform uint tid,
    uniform RWStructuredBuffer<float> buf)
{
    k(tid, buf);
}

// On the source targets, `k` is emitted as an ordinary function that the entry point calls.
// CHECK-HLSL-NOT: [numthreads
// CHECK-HLSL: {{^}}void [[K:k_[0-9]+]](
// CHECK-HLSL: [numthreads(32, 1, 1)]
// CHECK-HLSL-NEXT: void compute_main(
// CHECK-HLSL: [[K]](

// CHECK-CUDA-NOT: __global__ {{.*}}k_
// CHECK-CUDA: __device__ void [[K:k_[0-9]+]](
// CHECK-CUDA: extern "C" __global__ void compute_main(
// CHECK-CUDA: [[K]](

// CHECK-WGSL-NOT: @compute
// CHECK-WGSL: fn [[K:k_[0-9]+]](
// CHECK-WGSL: @compute
// CHECK-WGSL-NEXT: @workgroup_size(32, 1, 1)
// CHECK-WGSL-NEXT: fn compute_main(
// CHECK-WGSL: [[K]](

// CHECK-METAL-NOT: {{\[\[kernel\]\]}}
// CHECK-METAL: {{^}}void [[K:k_[0-9]+]](
// CHECK-METAL: {{\[\[kernel\]\]}} void compute_main(
// CHECK-METAL: [[K]](

// Exactly one LocalSize execution mode, targeting the entry point; none targets the callee `k`,
// which survives as an ordinary OpFunction.
// CHECK-SPV: OpEntryPoint GLCompute %[[EP:[A-Za-z0-9_]+]] "main"
// CHECK-SPV-NOT: OpExecutionMode {{%[A-Za-z0-9_]+}} LocalSize
// CHECK-SPV: OpExecutionMode %[[EP]] LocalSize
// CHECK-SPV-NOT: OpExecutionMode {{%[A-Za-z0-9_]+}} LocalSize
// CHECK-SPV: OpName %[[CALLEE:[A-Za-z0-9_]+]] "k"
// CHECK-SPV: %[[CALLEE]] = OpFunction
