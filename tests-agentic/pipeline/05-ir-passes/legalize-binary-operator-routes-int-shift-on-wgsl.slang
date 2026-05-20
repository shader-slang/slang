//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/llm-generated/pipeline/05-ir-passes.md#type-and-value-legalization
//META: doc_section_digest=440ef60cd49aa7b190c5138fc5695d86b4ac64644e03bf09eb6782e82ce75b0e
//META: purpose=The legalize-binary-operator pass adjusts a shift's RHS so it matches the target's signedness rules — on WGSL the shift count is a `u32`.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The "Legalize binary operator" row of the type-and-value
// legalization table (`slang-ir-legalize-binary-operator.cpp`)
// performs per-target binary-op legalization. WGSL requires the
// shift count to be `u32`; the emit shows a cast / conversion of
// the shift amount to the unsigned type.

//TEST:SIMPLE(filecheck=WGSL):-target wgsl -stage compute -entry main

uniform RWStructuredBuffer<int> buf;
uniform int a;
uniform int n;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = a << n;
}

// WGSL: fn main
// WGSL: u32(
