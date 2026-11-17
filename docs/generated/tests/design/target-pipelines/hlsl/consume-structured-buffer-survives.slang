//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#phase-b-specialization-and-type-legalization
//META: doc_section_digest=b1a772cd52d2040f30154ab240611958fb152ee42151e424dc0b33db4c29e97f
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
