//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/values.md#comparison
//META: doc_section_digest=a41eec816a55db301ccafcfa867bab048ae6ed92cc99c5f791c2e8474302d15d
//META: purpose=The `cmpLT` opcode lowers to three distinct SPIR-V ordering ops keyed by operand type: `OpSLessThan` for signed, `OpULessThan` for unsigned, `OpFOrdLessThan` for floating-point.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the single `cmpLT` opcode behind `<` fans out on
// operand type when it reaches SPIR-V. The SPIR-V
// legalizer branches the ordered relational on operand type: signed
// `OpSLessThan`, unsigned `OpULessThan`, float `OpFOrdLessThan`. This pins
// the distinguishing op per type row. GLSL renders all three as the `<`
// operator (no per-type token), so spirv-asm is the distinguishing
// target. Operands are uniforms; `select` keeps the bool result live.

//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -stage compute -entry main

uniform int sa;
uniform int sb;
uniform uint ua;
uniform uint ub;
uniform float fa;
uniform float fb;
uniform RWStructuredBuffer<int> ibuf;

[numthreads(1, 1, 1)]
void main()
{
    ibuf[0] = select(sa < sb, 1, 0);
    ibuf[1] = select(ua < ub, 1, 0);
    ibuf[2] = select(fa < fb, 1, 0);
}

// SPV-DAG: OpSLessThan %bool %{{[0-9]+}} %{{[0-9]+}}
// SPV-DAG: OpULessThan %bool %{{[0-9]+}} %{{[0-9]+}}
// SPV-DAG: OpFOrdLessThan %bool %{{[0-9]+}} %{{[0-9]+}}
