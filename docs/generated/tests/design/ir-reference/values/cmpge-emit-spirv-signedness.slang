//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ir-reference/values.md#comparison
//META: doc_section_digest=04ce765c6615b6d4afcb57668ffc53ea9c74c835ea05c4c4d58b93fe0cbf4541
//META: purpose=The `cmpGE` opcode lowers to three distinct SPIR-V ops keyed by operand type: `OpSGreaterThanEqual` for signed, `OpUGreaterThanEqual` for unsigned, `OpFOrdGreaterThanEqual` for floating-point.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Same per-type branch as cmplt-emit-spirv-signedness.slang but for the
// `>=` row of the Comparison table, confirming the signed/unsigned/float
// split is uniform across the relational family rather than specific to
// `<`. CHECK pins the distinguishing SPIR-V op per operand-type row.
// GLSL collapses to the `>=` operator; spirv-asm distinguishes.

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
    ibuf[0] = select(sa >= sb, 1, 0);
    ibuf[1] = select(ua >= ub, 1, 0);
    ibuf[2] = select(fa >= fb, 1, 0);
}

// SPV-DAG: OpSGreaterThanEqual %bool %{{[0-9]+}} %{{[0-9]+}}
// SPV-DAG: OpUGreaterThanEqual %bool %{{[0-9]+}} %{{[0-9]+}}
// SPV-DAG: OpFOrdGreaterThanEqual %bool %{{[0-9]+}} %{{[0-9]+}}
