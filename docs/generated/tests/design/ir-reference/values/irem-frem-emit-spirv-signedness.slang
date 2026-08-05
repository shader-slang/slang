//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/values.md#arithmetic-and-bitwise
//META: doc_section_digest=19b077f47f71a1feb81adb9dea6ff4919edb188c235b3634dce08c45cf359e3c
//META: purpose=The `irem` opcode lowers to `OpSRem` for signed and `OpUMod` for unsigned remainder, and `frem` lowers to `OpFRem` — three distinct SPIR-V ops keyed by operand type.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the two `%` opcodes — `irem` for integers and `frem`
// for floats — fan out further on the way to SPIR-V. The SPIR-V legalizer
// splits the integer form by signedness (signed `OpSRem` vs unsigned
// `OpUMod`) and emits `OpFRem` for the float form. This pins the
// distinguishing token per operand-type row. Operands are uniforms to
// defeat constant folding.

//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -stage compute -entry main

uniform int sa;
uniform int sb;
uniform uint ua;
uniform uint ub;
uniform float fa;
uniform float fb;
uniform RWStructuredBuffer<int> ibuf;
uniform RWStructuredBuffer<uint> ubuf;
uniform RWStructuredBuffer<float> fbuf;

[numthreads(1, 1, 1)]
void main()
{
    ibuf[0] = sa % sb;
    ubuf[0] = ua % ub;
    fbuf[0] = fmod(fa, fb);
}

// SPV-DAG: OpSRem %int %{{[0-9]+}} %{{[0-9]+}}
// SPV-DAG: OpUMod %uint %{{[0-9]+}} %{{[0-9]+}}
// SPV-DAG: OpFRem %float %{{[0-9]+}} %{{[0-9]+}}
