//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ir-reference/values.md#arithmetic-and-bitwise
//META: doc_section_digest=1d2f08dfecd3e2ccb26b50652f1294ebe4c1c5182a6aab140a589ccecd16faa1
//META: purpose=The `irem` opcode lowers to `OpSRem` for signed and `OpUMod` for unsigned remainder, and `frem` lowers to `OpFRem` — three distinct SPIR-V ops keyed by operand type.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc lists `irem` ("Integer remainder; *not* modulus") and `frem`
// ("Floating-point remainder") as the `%` lowering. The SPIR-V legalizer
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
