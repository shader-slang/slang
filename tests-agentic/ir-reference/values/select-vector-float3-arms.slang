//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=331a01d9d0cc721d8fc19a46fec17a4a275ba0e0
//META: doc_ref=docs/llm-generated/ir-reference/values.md#select
//META: doc_section_digest=8d30dda9ed7d2748e6be00027e0ec0b5dd193f2f9d741727f3dc3eb6d99acea4
//META: purpose=Expansion: `select(c, vec3, vec3)` with vector operands lowers to a single `select(%c, %a, %b)` with `Vec(Float, 3)` result — verifies the opcode handles aggregate operand pairs.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's `select` description allows any matching operand
// type pair. Vector operands surface the same opcode shape as
// scalar operands.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform bool c;
uniform float3 a;
uniform float3 b;
uniform RWStructuredBuffer<float3> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = select(c, a, b);
}

// IR-LABEL: func %main
// IR: select(%c, %a, %b)
