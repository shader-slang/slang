//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00Z
//META: source_commit=781ed4aac159d764c5350d082058d68d4f91b785
//META: doc_ref=docs/llm-generated/target-pipelines/metal.md#specializeaddressspaceformetal
//META: doc_section_digest=80c941477d24c10fff0c5bef4c1b8c2e3af5b3a9a05d65d62133e771249cc15e
//META: purpose=Stress: a 256-element `groupshared` array still gets the `threadgroup` address space annotation and emits as `array<int, int(256)> threadgroup*`.
//META: intent=stress
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Stress axis: large groupshared. The `specializeAddressSpaceForMetal`
// pass scales linearly with array size; we probe the 256-element
// case to confirm no pathological behavior.

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage compute

groupshared int sarr[256];
RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(64, 1, 1)]
void main(uint tid : SV_GroupIndex)
{
    sarr[tid] = int(tid);
    outBuf[tid] = sarr[tid];
}

// CHECK: array<int, int(256)> threadgroup*
