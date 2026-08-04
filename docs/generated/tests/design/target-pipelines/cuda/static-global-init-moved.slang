//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:40:21+00:00
//META: source_commit=4e394f5c941b8145021611e1d3016b1f27a4ba52
//META: doc_ref=docs/generated/design/target-pipelines/cuda.md#inlineglobalconstantsforlegalization-for-cuda
//META: doc_section_digest=429f7fe82b3dcfac3d26efec1b4c672716be1f4b213a8dcc4b88cc04f2e6b7e3
//META: purpose=CUDASource always runs inlineGlobalConstantsForLegalization (a final PTX request runs the pipeline as CUDASource and takes the same short-circuit; only CUDAHeader skips) so a static module-scope constant is inlined at its use sites as the literal value because nvrtc rejects dynamic __device__ initialization.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the CUDASource short-circuit for
// `inlineGlobalConstantsForLegalization` (`target == CUDASource ||
// (isCPUTarget && isKernelTarget) || ...`): because nvrtc rejects
// dynamic `__device__` initialization, the static constant's value
// must be inlined at the use site. The CHECKs assert the literal
// `int(5)` appears in the kernel and that no `__device__ int counter`
// global with an initializer is emitted. (`-target cuda` is the
// CUDASource arm the claim is scoped to; only CUDAHeader skips.)

//TEST:SIMPLE(filecheck=CHECK):-target cuda -entry main -stage compute

static const int counter = 5;

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = counter + int(tid.x);
}

// CHECK: extern "C" __global__ void main
// CHECK: int(5)
// CHECK-NOT: __device__ int counter
