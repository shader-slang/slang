//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00Z
//META: source_commit=781ed4aac159d764c5350d082058d68d4f91b785
//META: doc_ref=docs/llm-generated/target-pipelines/metal.md#specializeaddressspaceformetal
//META: doc_section_digest=80c941477d24c10fff0c5bef4c1b8c2e3af5b3a9a05d65d62133e771249cc15e
//META: purpose=Boundary: declaring multiple `groupshared` variables of different shapes (scalar, array, vector) — each independently picks up the `threadgroup` address space annotation.
//META: intent=boundary
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary axis: composition of groupshared variables of different
// element shapes. We verify multiple `threadgroup` annotations
// appear in the same kernel.

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage compute

groupshared int sval;
groupshared float farr[4];
groupshared int4 vec_arr[4];
RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(4, 1, 1)]
void main(uint tid : SV_GroupIndex)
{
    sval = int(tid);
    farr[tid] = float(tid);
    vec_arr[tid] = int4(int(tid));
    outBuf[tid] = sval + int(farr[tid]) + vec_arr[tid].x;
}

// CHECK: threadgroup
// CHECK: threadgroup
// CHECK: threadgroup
