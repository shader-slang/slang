//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T20:30:00+00:00
//META: source_commit=273b0c1d7bd60576ed4d24081c0d1b9f61952281
//META: doc_ref=docs/llm-generated/cross-cutting/ir-instructions.md#value-instructions
//META: doc_section_digest=ddd065b774b7979630d27ec19315b658865b6fb853c55bd6e6ee0a0aced77aba
//META: purpose=Negative probe of the `bitCast` conversion opcode: `bit_cast<T>` from a value whose size does not match `T` is a documented conversion-family error, rejected by type checking before IR lowering.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `bit_cast<T>` is the user surface for the documented `bitCast`
// opcode. Because `bitCast` preserves bit width rather than numeric
// value, a same-width destination type is required. Passing a scalar
// to a `bit_cast<uint4>` (a 4x wider destination) produces a
// front-end type-mismatch error before IR generation.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv -stage compute -entry main

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = bit_cast<uint4>(0u);
//CHECK:                    ^ type mismatch in expression
// CHECK: E30019
}
