//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ir-reference/values.md#arithmetic-and-bitwise
//META: doc_section_digest=1d2f08dfecd3e2ccb26b50652f1294ebe4c1c5182a6aab140a589ccecd16faa1
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
// CHECK: E30002
}
