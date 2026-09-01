//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/targets.md#profiles
//META: doc_section_digest=e5b9757f49d35cc72a9566431514fc5b19482718219a34a0a4240001ec32891e
//META: purpose=A cross-family request such as `-target spirv-asm -profile glsl_450` keeps the target keyhole populated by the target rather than the profile, so SPIR-V is emitted and no GLSL translation unit appears.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the target keyhole is chosen from `-target` first and
// the profile's atoms are only admitted where compatible, so pairing
// a GLSL-family profile with the direct SPIR-V target does not yield
// a set carrying both `glsl_450` and `spirv`. The compile is asked
// for SPIR-V assembly while the profile names GLSL 450; the CHECKs
// assert a SPIR-V module header plus a SPIR-V entry point, and
// CHECK-NOT asserts no `#version` line — the marker GLSL text would
// have carried if the profile had won the keyhole.

//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -profile glsl_450 -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = 7;
}

// SPIRV: ; SPIR-V
// SPIRV: OpEntryPoint GLCompute %main
// SPIRV-NOT: #version
