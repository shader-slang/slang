//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=331a01d9d0cc721d8fc19a46fec17a4a275ba0e0
//META: doc_ref=docs/llm-generated/ir-reference/values.md#arithmetic-and-bitwise
//META: doc_section_digest=300bbe3c39d84a3dd7997f5a048fb4bd6e896b95057fa5fb8b7116c32840a6b7
//META: purpose=Negative boundary: integer `a / 0` is rejected by semantic check with "divide by zero" — no `div` IR opcode is produced. (Float divide-by-zero is permitted; see `div-float-by-positive-zero-uniform.slang`.)
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc lists `div` for `/` and keys the integer/float choice
// by operand type. On integers, dividing by the zero literal is
// undefined behavior and Slang rejects it at semantic check
// rather than lowering it to a `div` opcode.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = a / 0;
//CHECK:       ^ divide by zero
}
