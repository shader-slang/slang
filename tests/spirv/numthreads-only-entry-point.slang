//TEST:SIMPLE(filecheck=CHECK): -target spirv -emit-spirv-directly
//TEST:SIMPLE(filecheck=MULTI): -target spirv -fvk-use-entrypoint-name -emit-spirv-directly

// `[numthreads]` is by itself enough to mark a compute entry point: entry-point discovery infers the
// stage from the attribute, so a file like this compiles with no `[shader(...)]` and no `-entry` on
// the command line. Such a function is a real entry point, so it keeps its `LocalSize` execution
// mode -- the entry-point gate in the SPIR-V emitter must not mistake it for an ordinary callee.

// CHECK-DAG: OpEntryPoint GLCompute %computeMain
// CHECK-DAG: OpExecutionMode %computeMain LocalSize 64 1 1

// Several of them in one module each keep their own mode.
// MULTI-DAG: OpExecutionMode %mainA LocalSize 64 1 1
// MULTI-DAG: OpExecutionMode %mainB LocalSize 32 2 1
// MULTI-DAG: OpExecutionMode %mainC LocalSize 8 8 1

RWStructuredBuffer<float> buf;

[numthreads(64, 1, 1)]
void computeMain(uint tid: SV_DispatchThreadID)
{
    buf[tid] = tid;
}

[numthreads(64, 1, 1)]
void mainA(uint tid: SV_DispatchThreadID)
{
    buf[tid] = tid;
}

[numthreads(32, 2, 1)]
void mainB(uint tid: SV_DispatchThreadID)
{
    buf[tid] = tid * 2;
}

[numthreads(8, 8, 1)]
void mainC(uint tid: SV_DispatchThreadID)
{
    buf[tid] = tid * 3;
}
