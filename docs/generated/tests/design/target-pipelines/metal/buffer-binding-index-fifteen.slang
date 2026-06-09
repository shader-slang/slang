//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00Z
//META: source_commit=781ed4aac159d764c5350d082058d68d4f91b785
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#phase-d-metal-emit-and-downstream-tools
//META: doc_section_digest=15e31694fd5ee22d184f1b99f92912686150441a720712b25e415c394b7ce97f
//META: purpose=Stress: with 16 buffer parameters the Metal emitter still assigns sequential positional `[[buffer(N)]]` indices up through 15 (no Metal slot-budget compression).
//META: intent=stress
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Stress axis: binding-index pressure. The Metal emitter assigns
// `[[buffer(0)]] .. [[buffer(15)]]` for sixteen buffers. The
// positional rule (declaration order) still holds; the last
// declared buffer in the function signature is the RW one.

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage compute

StructuredBuffer<int> b0;
StructuredBuffer<int> b1;
StructuredBuffer<int> b2;
StructuredBuffer<int> b3;
StructuredBuffer<int> b4;
StructuredBuffer<int> b5;
StructuredBuffer<int> b6;
StructuredBuffer<int> b7;
StructuredBuffer<int> b8;
StructuredBuffer<int> b9;
StructuredBuffer<int> b10;
StructuredBuffer<int> b11;
StructuredBuffer<int> b12;
StructuredBuffer<int> b13;
StructuredBuffer<int> b14;
RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = b0[tid.x] + b1[tid.x] + b2[tid.x] + b3[tid.x]
                  + b4[tid.x] + b5[tid.x] + b6[tid.x] + b7[tid.x]
                  + b8[tid.x] + b9[tid.x] + b10[tid.x] + b11[tid.x]
                  + b12[tid.x] + b13[tid.x] + b14[tid.x];
}

// CHECK: buffer(15)
// CHECK-SAME: buffer(0)
// CHECK-SAME: buffer(14)
