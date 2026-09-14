//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/target-pipelines/cuda.md#cuda-target-pipeline
//META: doc_section_digest=7f28c727cc7d470e247b44739e49006942c2a76990f403c3d6e62db0f4f2684b
//META: purpose=CUDASource is spelled -target cuda or -target cu on the slangc command line, and the cu alias selects the same CUDASource pipeline and emits the same extern "C" __global__ kernel.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the alias half of the documented CLI spelling for
// CodeGenTarget::CUDASource. The enum name is not derivable from
// the flag, and `cu` is the shorter accepted spelling. The test
// compiles one shader under `-target cu` and asserts the artifact
// carries the CUDA prelude include and an `extern "C" __global__`
// kernel — the Phase D fingerprint of the CUDASource pipeline —
// which is only reachable if `cu` resolved to CUDASource rather
// than being rejected as an unknown target.

//TEST:SIMPLE(filecheck=CHECK):-target cu -entry main -stage compute

RWStructuredBuffer<int> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outputBuffer[tid.x] = int(tid.x) + 1;
}

// CHECK: slang-cuda-prelude.h
// CHECK: extern "C" __global__ void main{{.*}}()
