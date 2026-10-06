//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/values.md#select
//META: doc_section_digest=6a246596e363472d4dea3a9f4c80799257cdcad05d5bbe71a9d627181a6e8654
//META: purpose=Expansion: `select(c, vec3, vec3)` with vector operands lowers to a single `select(%c, %a, %b)` with `Vec(Float, 3)` result — verifies the opcode handles aggregate operand pairs.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `select` allows any matching operand
// type pair. Vector operands surface the same opcode shape as
// scalar operands.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o - -stage compute -entry main

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
