//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/cuda.md#undoparametercopy-and-transformparamstoconstref
//META: doc_section_digest=2daf73e31e7ca500dd32820aabeebff047d750c36d45ddc66ec8d467f09ff675
//META: purpose=undoParameterCopy rewrites explicit inout copy-in copy-out wrappers as pass-by-pointer on CUDA emit.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// CUDA shares the `undoParameterCopy` arm with CPU and Metal at
// line ~2041. The result for an `inout` parameter is a pointer-form
// callee signature; the caller passes the address of the local.

//TEST:SIMPLE(filecheck=CHECK):-target cuda -entry main -stage compute

void bump(inout int x, int v) { x = x + v; }

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int s = int(tid.x);
    bump(s, int(tid.y));
    outBuf[tid.x] = s;
}

// CHECK: bump{{[_0-9]*}}(int *
