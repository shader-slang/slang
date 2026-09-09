//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#phase-b-specialization-and-type-legalization
//META: doc_section_digest=bae02bfd9f0c35f37e81a96d88e88829e442c28a56e0f30d0bb769febf165f85
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
