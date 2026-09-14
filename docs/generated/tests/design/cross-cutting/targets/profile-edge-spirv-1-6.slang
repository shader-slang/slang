//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/targets.md#profiles
//META: doc_section_digest=e5b9757f49d35cc72a9566431514fc5b19482718219a34a0a4240001ec32891e
//META: purpose=The highest documented SPIR-V profile (`spirv_1_6` — top of the SPIRV family in slang-profile-defs.h) compiles a vanilla compute shader; the emitted module declares `Version: 1.6`.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Profile upper-edge probe for the SPIR-V family. `spirv_1_6` is the
// top of the SPIR-V PROFILE_VERSION list in slang-profile-defs.h.
// Mirror of `profile-edge-spirv-1-0.slang` at the high end; together
// they bracket the Family + Version axis the doc's Profiles section
// describes.

//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -profile spirv_1_6 -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = 1;
}

// SPIRV: Version: 1.6
// SPIRV: OpEntryPoint GLCompute %main
