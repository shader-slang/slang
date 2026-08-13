//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/wgsl.md#phase-c-wgsl-legalization-lowering-phi-elimination
//META: doc_section_digest=e9f2e62d5a5d7efaa803abbf9f002d3308a6da2c7e2ea7013076a2b7c23eb4c4
//META: purpose=collectCooperativeMetadata is gated on cooperative_matrix or cooperative_vector capability and does not fire for a plain WGSL compute kernel
//META: intent=negative
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Phase C row 28 says collectCooperativeMetadata only runs when
// targetCaps implies cooperative_matrix or cooperative_vector. A
// plain WGSL compute kernel has neither capability, so the pass
// is skipped and no cooperative-matrix / coopvec spelling appears
// in the emitted WGSL text.

//TEST:SIMPLE(filecheck=CHECK):-target wgsl -entry main -stage compute

RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(8,1,1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    float acc = 0.0;
    for (int i = 0; i < 4; ++i)
    {
        acc += float(tid.x + uint(i));
    }
    outBuf[tid.x] = acc;
}

// Sanity: the normal compute emit shape is present.
// CHECK: @compute
// CHECK: @workgroup_size(8, 1, 1)
// CHECK: fn main
// No cooperative-matrix / cooperative-vector metadata leaks into
// the WGSL emit, because collectCooperativeMetadata never ran.
// CHECK-NOT: CooperativeMatrix
// CHECK-NOT: coopmat
// CHECK-NOT: cooperative_matrix
// CHECK-NOT: cooperative_vector
// CHECK-NOT: coopvec
