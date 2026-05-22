//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T21:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/ast-reference/expressions.md#nodes
//META: doc_section_digest=5cae24401ed06bd3567242934951f344da434f10804637bdd679d55ab6b5bb2b
//META: purpose=An InitializerListExpr with more elements than the aggregate has fields is rejected with a "too many initializers" diagnostic.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Negative shape for InitializerListExpr count-mismatch on the
// upper side: a 3-field struct receives 5 elements; the checker
// must diagnose the overflow.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

struct R { int a; int b; int c; }

void main()
{
    R r = {1, 2, 3, 4, 5};
//CHECK:  ^ too many initializers in initializer list
//CHECK:  ^ too many initializers (expected 3, got 5)
// CHECK: E30523
}
