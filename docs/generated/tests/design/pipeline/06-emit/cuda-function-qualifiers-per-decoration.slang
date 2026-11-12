//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T20:49:04+00:00
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/pipeline/06-emit.md#cuda
//META: doc_section_digest=727deb3a627e0080c2b0beee98e62ea785941ba3a3f746a00146314b79eaeaee
//META: purpose=The CUDA emitter qualifies a compute entry point extern "C" __global__, a non-entry function carrying a CUDA kernel decoration __global__, one carrying a host decoration __host__, and every other function __device__.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the per-function qualifier rule of emitFunctionPreambleImpl. Four
// functions are emitted: the compute entry point, a [CudaKernel] helper, a
// [CudaHost] helper, and an ordinary helper. Each CHECK-DAG pins the
// qualifier together with the function name so a qualifier swap between two
// functions is caught. The ordinary helper is matched as `__device__ int`
// to confirm it is not promoted to a kernel or host function.

//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[CudaKernel]
void kern(uint3 t)
{
    outBuf[t.x] = 1;
}

[CudaHost]
int hostFn(int v)
{
    return v + 1;
}

int plain(int v)
{
    return v * 2;
}

[shader("compute")]
[numthreads(4, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = plain(hostFn(int(tid.x)));
}

// CUDA-DAG: __host__ int hostFn(
// CUDA-DAG: __device__ int plain_{{[0-9]+}}(
// CUDA-DAG: extern "C" __global__ void main_{{[0-9]+}}()
// CUDA-DAG: __global__ void kern(
