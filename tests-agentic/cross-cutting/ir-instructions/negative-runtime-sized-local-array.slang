//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T12:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/llm-generated/cross-cutting/ir-instructions.md#type-instructions
//META: doc_section_digest=1776a4feb6a9dd1e193d21a732a0f0b76a7e29c1947a97e582c46ac76f6527b8
//META: purpose=Negative probe of the documented `Array(elementType, elementCount)` type opcode: the `elementCount` operand must be a compile-time constant, so a runtime-sized local array is rejected before IR lowering.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's `Array` type opcode has shape `(elementType,
// elementCount)`. `elementCount` is a constant operand by contract,
// not a runtime value. Declaring `int arr[n]` with a `uniform` `n`
// therefore fails the constant-evaluation requirement at type-check
// time — the front end never reaches IR lowering to build an
// `Array(...)`.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv -stage compute -entry main

uniform int n;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int arr[n];
//CHECK:    ^ expression does not evaluate to a compile-time constant
    buf[0] = arr[0];
}
