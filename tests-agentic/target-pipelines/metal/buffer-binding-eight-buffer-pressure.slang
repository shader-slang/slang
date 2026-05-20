//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00Z
//META: source_commit=781ed4aac159d764c5350d082058d68d4f91b785
//META: doc_ref=docs/llm-generated/target-pipelines/metal.md#phase-d-metal-emit-and-downstream-tools
//META: doc_section_digest=15e31694fd5ee22d184f1b99f92912686150441a720712b25e415c394b7ce97f
//META: purpose=Stress: with 8+ buffer parameters the Metal emitter still assigns positional `[[buffer(N)]]` indices and produces a KernelContext struct holding all of them.
//META: intent=stress
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Stress axis: 8+ buffer params force a KernelContext indirection
// and exercise the binding-index pressure path. The RW buffer is
// the entry-point parameter and ends up at `buffer(8)` (highest
// index), while `b0..b7` occupy `buffer(0)..buffer(7)`.

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage compute

StructuredBuffer<int> b0;
StructuredBuffer<int> b1;
StructuredBuffer<int> b2;
StructuredBuffer<int> b3;
StructuredBuffer<int> b4;
StructuredBuffer<int> b5;
StructuredBuffer<int> b6;
StructuredBuffer<int> b7;
RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = b0[tid.x] + b1[tid.x] + b2[tid.x] + b3[tid.x]
                  + b4[tid.x] + b5[tid.x] + b6[tid.x] + b7[tid.x];
}

// CHECK: struct KernelContext_0
// CHECK: buffer(8)
// CHECK-SAME: buffer(0)
// CHECK-SAME: buffer(7)
