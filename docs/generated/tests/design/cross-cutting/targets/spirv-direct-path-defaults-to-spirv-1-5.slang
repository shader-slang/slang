//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/targets.md#profiles
//META: doc_section_digest=e5b9757f49d35cc72a9566431514fc5b19482718219a34a0a4240001ec32891e
//META: purpose=On the direct SPIR-V path with no `-profile` supplying a SPIR-V version, the target capability set falls back to `spirv_1_5` and the emitted module declares `Version: 1.5`.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the documented fallback that applies when the target's
// capability set is built with no SPIR-V version atom to copy out of
// a profile: the direct SPIR-V path substitutes `spirv_1_5`. The test
// compiles a plain compute kernel with no `-profile` at all — the
// lower edge of "profile supplies none" — and CHECK pins the version
// word the assembler prints in the module header, which is the only
// user-visible rendering of the version atom the set ended up with.

//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = 1;
}

// SPIRV: Version: 1.5
// SPIRV: OpEntryPoint GLCompute %main
