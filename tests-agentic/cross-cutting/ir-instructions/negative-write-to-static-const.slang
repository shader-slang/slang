//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T12:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/llm-generated/cross-cutting/ir-instructions.md#function-and-module-structure
//META: doc_section_digest=3cd430fe84a5b7025d059358578317eab126613a22446f8d9d7f776e96724317
//META: purpose=Negative probe of the documented `globalConstant` opcode: a `static const` declaration is read-only by definition, so assigning to it is rejected at type-check time before any IR is built.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `globalConstant` is the documented module-scope storage opcode for
// `static const` values. By contract its IR result is read-only;
// the user-facing consequence is that an assignment expression with
// the constant on the LHS is rejected by the front end as not being
// an l-value, never reaching IR lowering.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv -stage compute -entry main

static const int k = 42;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    k = 100;
/*CHECK:
      ^ left of '=' is not an l-value
*/
// CHECK: E30011
    buf[0] = k;
}
