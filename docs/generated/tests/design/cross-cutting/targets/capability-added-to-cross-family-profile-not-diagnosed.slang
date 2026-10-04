//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/targets.md#profiles-versus-explicit--capability
//META: doc_section_digest=aa5b75d19e741d5bc36e7de273e6e9a576a676ed130d76b894b28a5c65ebc8e9
//META: purpose=When the `-profile` pins no version of the target's version family, a `-capability` that raises the emitted version is not diagnosed as a conflict and the raised version is emitted.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the carve-out on the capability-versus-profile conflict
// check: the check only fires when the profile actually pins a
// version in the family the target emits. Here the SPIR-V target is
// paired with a GLSL-family profile, so nothing in the SPIR-V family
// is pinned, and `-capability spirv_1_6` is free to raise the emitted
// version. The CHECK pins `Version: 1.6`, which shows both that the
// capability took effect and that the compile was not stopped by a
// conflict diagnostic.

//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -profile glsl_450 -capability spirv_1_6 -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = 9;
}

// SPIRV: Version: 1.6
// SPIRV: OpEntryPoint GLCompute %main
