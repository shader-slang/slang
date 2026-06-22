//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T18:00:00+00:00
//META: source_commit=690f6a3084801386b77186394e0f6e8c120824a4
//META: doc_ref=docs/generated/design/cross-cutting/ir-instructions.md#value-instructions
//META: doc_section_digest=ddd065b774b7979630d27ec19315b658865b6fb853c55bd6e6ee0a0aced77aba
//META: purpose=Comparing `int8_t < int16_t` lowers to an `intCast` promoting the narrower operand followed by `cmpLT` over the unified width — pins the mixed-narrow-type promotion path of the comparison family.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc names `cmpLT` without stating how mixed-precision operands
// reach a single comparison. The lowering inserts an explicit
// `intCast` on the narrower operand before the comparison runs, with
// both opcodes named by the doc's value-instructions row.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int8_t a;
uniform int16_t b;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = (a < b) ? 1 : 0;
}

// IR-LABEL: func %main
// IR: intCast(%a)
// IR: cmpLT(%{{.*}}, %b)
