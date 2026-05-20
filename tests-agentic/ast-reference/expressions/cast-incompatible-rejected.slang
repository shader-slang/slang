//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:30:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/ast-reference/expressions.md#nodes
//META: doc_section_digest=5cae24401ed06bd3567242934951f344da434f10804637bdd679d55ab6b5bb2b
//META: purpose=An ExplicitCastExpr `(T) e` with no available conversion from typeof(e) to T is rejected with a type-mismatch diagnostic.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// ExplicitCastExpr derives from TypeCastExpr, which is an InvokeExpr
// of a conversion function. When no conversion exists from a struct
// type to int, the checker reports a type-mismatch diagnostic.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

struct S { int a; }

void main()
{
    S s;
    int x = (int) s;
//CHECK:          ^ type mismatch in expression
//CHECK:          ^ expected an expression of type 'int', got 'S'
}
