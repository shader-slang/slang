//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/values.md#conversions
//META: doc_section_digest=15a849f7cabcf06b95570121d08d30f735912628e8a357779f17fa39de0aca01
//META: purpose=Expansion: widest float-narrowing `double -> half` lowers to a single `floatCast(%da)` with `Half` result.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Widest precision-narrowing step in the float ladder, the
// reverse of the existing `half -> double` widening boundary.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform double da;
uniform RWStructuredBuffer<half> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = half(da);
}

// IR-LABEL: func %main
// IR: floatCast(%da)
