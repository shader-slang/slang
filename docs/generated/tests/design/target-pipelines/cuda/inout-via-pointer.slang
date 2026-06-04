//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:30:46+00:00
//META: source_commit=330c9a8d807b9f9352e4754f466d1244ae681cff
//META: doc_ref=docs/generated/design/target-pipelines/cuda.md#undoparametercopy-and-transformparamstoconstref
//META: doc_section_digest=de89b59326004a5e5540df99bb073fc4604adfca7c7993a97eeeb53f850f6575
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
