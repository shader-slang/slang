//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/values.md#select
//META: doc_section_digest=e40d6712a2443477b9dd87fd4146f5ac3852042d7d571c4f7aa5a1ef274fdbc1
//META: purpose=Expansion: `select(c, vec3, vec3)` with vector operands lowers to a single `select(%c, %a, %b)` with `Vec(Float, 3)` result — verifies the opcode handles aggregate operand pairs.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `select` allows any matching operand
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
