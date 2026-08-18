//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/targets.md#definition-forms
//META: doc_section_digest=d6d360ed810fe20f1543f761a89b7a194383364a9db523edf2e823b33ddfb19e
//META: purpose=The `spirv_latest` alias names the highest member of the SPIR-V version family, so `-capability spirv_latest` emits the same module version as the explicit `spirv_1_6` spelling.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a version family's `*_latest` alias resolves to that
// family's highest version atom rather than being an independent
// name. The same kernel is compiled twice — once asking for
// `spirv_latest` and once for the explicit top-of-chain `spirv_1_6` —
// and both CHECKs pin `Version: 1.6` in the module header. Matching
// headers is what shows the alias is a second spelling of the same
// atom; if it named anything lower the LATEST run would print a
// smaller version.

//TEST:SIMPLE(filecheck=LATEST):-target spirv-asm -capability spirv_latest -entry main -stage compute
//TEST:SIMPLE(filecheck=EXPLICIT):-target spirv-asm -capability spirv_1_6 -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = 3;
}

// LATEST: Version: 1.6
// EXPLICIT: Version: 1.6
