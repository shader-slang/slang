//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ir-reference/values.md#arithmetic-and-bitwise
//META: doc_section_digest=1d2f08dfecd3e2ccb26b50652f1294ebe4c1c5182a6aab140a589ccecd16faa1
//META: purpose=The unified `div` IR opcode lowers to three distinct SPIR-V ops keyed by operand type: `OpSDiv` for signed, `OpUDiv` for unsigned, `OpFDiv` for floating-point.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc states `div` is "Division (signed / unsigned / floating-point
// keyed by operand types)". The IR opcode is one (`div`); the SPIR-V
// legalizer branches it into three distinct ops. This pins the
// distinguishing emitted token per operand-type row of that branch.
// Operands come from uniforms so the constant folder cannot collapse the
// division. GLSL collapses all three rows to the `/` operator, so the
// distinguishing target for this per-type branch is spirv-asm.

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
    ibuf[0] = sa / sb;
    ubuf[0] = ua / ub;
    fbuf[0] = fa / fb;
}

// SPV-DAG: OpSDiv %int %{{[0-9]+}} %{{[0-9]+}}
// SPV-DAG: OpUDiv %uint %{{[0-9]+}} %{{[0-9]+}}
// SPV-DAG: OpFDiv %float %{{[0-9]+}} %{{[0-9]+}}
