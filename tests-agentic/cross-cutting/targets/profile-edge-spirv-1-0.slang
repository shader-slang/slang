//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=60db9042207670b39b0524118887efacb7a3d42d
//META: doc_ref=docs/llm-generated/cross-cutting/targets.md#profiles
//META: doc_section_digest=40b4f2fae77db035dad60bd27fe1b81fe055b218323e5afdfe04adf91d306631
//META: purpose=The lowest documented SPIR-V profile (`spirv_1_0` — floor of the SPIRV family in slang-profile-defs.h) compiles a vanilla compute shader; the emitted module declares `Version: 1.0`.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Profile lower-edge probe for the SPIR-V family. `spirv_1_0` is the
// floor of the SPIR-V PROFILE_VERSION list in slang-profile-defs.h.
// The Profiles section's example uses `-target spirv -profile glsl_450`
// to show profile drives capability sets; this test extends that to a
// SPIR-V-native profile spelling at its lowest version. The compiler
// emits a "version too old" warning but still produces a Version 1.0
// SPIR-V module.

//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -profile spirv_1_0 -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = 1;
}

// SPIRV: Version: 1.0
// SPIRV: OpEntryPoint GLCompute %main
