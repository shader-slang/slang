//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/generated/design/cross-cutting/targets.md#profiles
//META: doc_section_digest=40b4f2fae77db035dad60bd27fe1b81fe055b218323e5afdfe04adf91d306631
//META: purpose=The doc's Profiles section gives `-target spirv -profile glsl_450` as an example that selects a profile-driven capability set. A profile that pairs SPIR-V with GLSL 450 parses and compiles a vanilla compute shader.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The Profiles section uses `-target spirv -profile glsl_450` as the
// motivating example. We verify the combo parses and lowers to a
// compute SPIR-V module — i.e. the resulting CapabilitySet has both
// the glsl_450 atoms and the spirv target keyhole. The marker we
// pin is the SPIR-V module header (`OpCapability Shader`) plus the
// compute entry-point execution mode.

//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -profile glsl_450 -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(2, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = 1;
}

// SPIRV: OpCapability Shader
// SPIRV: OpEntryPoint GLCompute
// SPIRV: OpExecutionMode %main LocalSize 2 1 1
