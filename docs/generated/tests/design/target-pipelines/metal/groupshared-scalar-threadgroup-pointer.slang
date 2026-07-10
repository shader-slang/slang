//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00Z
//META: source_commit=781ed4aac159d764c5350d082058d68d4f91b785
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#specializeaddressspaceformetal
//META: doc_section_digest=315445765d1752992223a6aff4210e62fdeddf94bac693f294830da1675618e3
//META: purpose=Boundary: a scalar `groupshared int` element is annotated with the `threadgroup` address space (not just arrays of `groupshared`).
//META: intent=boundary
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary axis: groupshared scalar. The KernelContext field uses
// `int threadgroup*` and the kernel body declares a
// `threadgroup int` local; the address space sticks to the scalar
// case, not just arrays.

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage compute

groupshared int sval;
RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    sval = int(tid.x);
    outBuf[tid.x] = sval;
}

// CHECK: int threadgroup*
// CHECK: threadgroup int
