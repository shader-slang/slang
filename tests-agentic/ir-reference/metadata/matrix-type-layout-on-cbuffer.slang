//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:50:00Z
//META: source_commit=2aa9f69f5e2e75f6e2f4231a451a1a022818e18b
//META: doc_ref=docs/llm-generated/ir-reference/metadata.md#layout-family
//META: doc_section_digest=295e5ac09f4b7f77dd6669103c943586264fcbb5b6e2283e5226b00e48c0393b
//META: purpose=A matrix-typed cbuffer field induces a matrixTypeLayout opcode whose first operand encodes row-major vs column-major.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Per the Layout-family table: `matrixTypeLayout` encodes the matrix
// layout (variadic, min=1, hoistable). Its leading integer operand
// records row-major-vs-column-major; the value is target-dependent
// and pinned in the test only as `{{[0-9]+}}`.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

cbuffer CB
{
    float4x4 m;
}

uniform RWStructuredBuffer<float4> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = mul(m, float4(0, 0, 0, 1));
}

// CHECK: matrixTypeLayout({{[0-9]+}} : Int
