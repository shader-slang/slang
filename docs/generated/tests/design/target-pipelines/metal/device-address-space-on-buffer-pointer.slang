//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:50:00Z
//META: source_commit=2aa9f69f5e2e75f6e2f4231a451a1a022818e18b
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#specializeaddressspaceformetal
//META: doc_section_digest=315445765d1752992223a6aff4210e62fdeddf94bac693f294830da1675618e3
//META: purpose=Structured-buffer storage pointers carry Metal's `device` address space; constant-buffer pointers carry `constant`. Both come from specializeAddressSpaceForMetal.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// On Metal, RW/StructuredBuffer storage pointer arguments live
// in the `device` address space, while cbuffer / uniform
// parameter pointers live in `constant`. The address-space
// qualifier appears between the element type and the `*`.

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage compute

uniform int scale;
RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = int(tid.x) * scale;
}

// CHECK: int device*
// CHECK: constant*
