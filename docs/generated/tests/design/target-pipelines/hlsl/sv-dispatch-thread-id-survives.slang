//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#phase-d-hlsl-emit-and-downstream-tools
//META: doc_section_digest=c7c97cf6f5a8dc2bac27a37c7292d84bc70e3e1764ddec58018711cf9fd606ce
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
