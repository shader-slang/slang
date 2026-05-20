//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00Z
//META: source_commit=781ed4aac159d764c5350d082058d68d4f91b785
//META: doc_ref=docs/llm-generated/target-pipelines/metal.md#specializeaddressspaceformetal
//META: doc_section_digest=80c941477d24c10fff0c5bef4c1b8c2e3af5b3a9a05d65d62133e771249cc15e
//META: purpose=Boundary: a groupshared array of vector type (`float4`) still emits with `threadgroup` address space on `array<float4, int(8)>`.
//META: intent=boundary
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary axis: groupshared element type = vector. Beyond
// scalar/struct, vector-typed groupshared also picks up the
// `threadgroup` address space annotation.

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage compute

groupshared float4 vec_arr[8];
RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(8, 1, 1)]
void main(uint tid : SV_GroupIndex)
{
    vec_arr[tid] = float4(float(tid));
    outBuf[tid] = vec_arr[tid];
}

// CHECK: array<float4, int(8)> threadgroup*
