// Regression test for https://github.com/shader-slang/slang/issues/12392:
// compiling an entry point (`compute_main`) that calls a function which is itself an entry point
// (`k`, imported from `inner` and marked `[shader("compute")]` but not selected here) used to crash
// in a post-link IR pass. It must compile with `k` treated as an ordinary callee.

//TEST:SIMPLE(filecheck=CHECK-HLSL): -target hlsl -entry compute_main -stage compute
// The SPIR-V cell uses -O0 so `k` is not inlined away, keeping it as an ordinary callee in the
// output. That is what exercises #12397: a called `[numthreads]` function must NOT get an
// entry-point-scoped `OpExecutionMode ... LocalSize`; only the selected entry point may.
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

// CHECK-HLSL: void compute_main

// Exactly one LocalSize execution mode, targeting the entry point; none targets the callee `k`,
// which survives as an ordinary OpFunction.
// CHECK-SPV: OpEntryPoint GLCompute %[[EP:[A-Za-z0-9_]+]] "main"
// CHECK-SPV: OpExecutionMode %[[EP]] LocalSize
// CHECK-SPV-NOT: OpExecutionMode {{%[A-Za-z0-9_]+}} LocalSize
// CHECK-SPV: OpName %[[CALLEE:[A-Za-z0-9_]+]] "k"
// CHECK-SPV: %[[CALLEE]] = OpFunction
