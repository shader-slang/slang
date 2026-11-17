//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#specializeaddressspaceformetal
//META: doc_section_digest=59ef424a18f99049f9ec229ab276693c12e8cd63b80fe7453b5a039912306594
//META: purpose=Stress: a 256-element `groupshared` array still receives the `threadgroup` address space and emits as `array<int, int(256)> threadgroup*`.
//META: intent=stress
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Pushes the address-space assignment at a large array bound to confirm it
// is driven by the variable's storage class rather than by anything that
// scales with element count. CHECK asserts the pointer form
// `array<int, int(256)> threadgroup*` reaches emit, which also pins that
// the array bound survived the array-specialization passes intact.

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
