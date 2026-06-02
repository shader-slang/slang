//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#phase-b-specialization-and-type-legalization
//META: doc_section_digest=9833b4c0a727318f2e9ddb19a0e7c49fb0434ebb154535e0c52405c196a4547c
//META: purpose=Boundary: a standalone ConsumeStructuredBuffer survives HLSL emit with `.Consume()` method intact.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary axis: only `ConsumeStructuredBuffer` used (no Append).
// The HLSL emitter must keep the type name and the `.Consume()`
// method.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

ConsumeStructuredBuffer<int> consumeBuf;
RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = consumeBuf.Consume();
}

// CHECK: ConsumeStructuredBuffer<int{{.*}}> consumeBuf_{{[0-9]+}}
// CHECK: .Consume()
