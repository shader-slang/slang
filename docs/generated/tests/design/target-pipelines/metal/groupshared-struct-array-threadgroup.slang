//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00Z
//META: source_commit=781ed4aac159d764c5350d082058d68d4f91b785
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#specializeaddressspaceformetal
//META: doc_section_digest=315445765d1752992223a6aff4210e62fdeddf94bac693f294830da1675618e3
//META: purpose=Boundary: an array of a user struct stored as groupshared emits with the `threadgroup` address space on the array pointer — `specializeAddressSpaceForMetal` reaches user-defined element types.
//META: intent=boundary
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary axis: groupshared element type = user struct. The
// emitted KernelContext field becomes
// `array<S_0, int(4)> threadgroup*`.

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage compute

struct S { int a; float b; };

groupshared S sdata[4];
RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(4, 1, 1)]
void main(uint tid : SV_GroupIndex)
{
    sdata[tid].a = int(tid);
    sdata[tid].b = float(tid);
    outBuf[tid] = sdata[tid].a;
}

// CHECK: array<S_0, int(4)> threadgroup*
