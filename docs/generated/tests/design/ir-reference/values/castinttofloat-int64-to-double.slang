//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/values.md#conversions
//META: doc_section_digest=fd36e77aadaffc9bfb74e35c83d2c6e2a74c726d09922af2aa3a95ce43fcac0c
//META: purpose=Expansion: widest int-to-float `int64_t -> double` lowers to a single `castIntToFloat(%i64a)` with `Double` result.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Top-of-rank int-to-float surface (64-bit signed to 64-bit
// float). The doc places this under the same `castIntToFloat`
// opcode as the 32-bit cases.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o - -stage compute -entry main

uniform int64_t i64a;
uniform RWStructuredBuffer<double> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = double(i64a);
}

// IR-LABEL: func %main
// IR: castIntToFloat(%i64a)
