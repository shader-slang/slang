//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/values.md#conversions
//META: doc_section_digest=e048ee63e8d3331531d544e35ff8c335c38d075b516814b79f5d10d0602ac72a
//META: purpose=Expansion: `int -> half` lowers directly to `castIntToFloat(%a)` with `Half` result — no intermediate `float` step at IR level.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Tests that int-to-half does not synthesize an intermediate
// int-to-float-then-float-to-half pair; the doc places it under
// one `castIntToFloat` opcode keyed by the result float type.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<half> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = half(a);
}

// IR-LABEL: func %main
// IR: castIntToFloat(%a)
