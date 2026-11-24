//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#phase-d-hlsl-emit-and-downstream-tools
//META: doc_section_digest=affc65088b4faddcbee9b222f5a894089c99cb8eee13a5f0c0bac210c0fd5c78
//META: purpose=The SV_DispatchThreadID system-value semantic on a compute-stage input parameter survives the pipeline and appears on the HLSL entry point parameter list.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The compute system value `SV_DispatchThreadID` is one of the
// well-known HLSL semantics that the emit stage threads through
// from Slang into the HLSL parameter signature.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

RWStructuredBuffer<uint3> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[0] = tid;
}

// CHECK: uint3 tid_{{[0-9]+}} : SV_DispatchThreadID
