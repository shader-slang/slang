//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/targets.md#profiles
//META: doc_section_digest=e5b9757f49d35cc72a9566431514fc5b19482718219a34a0a4240001ec32891e
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
