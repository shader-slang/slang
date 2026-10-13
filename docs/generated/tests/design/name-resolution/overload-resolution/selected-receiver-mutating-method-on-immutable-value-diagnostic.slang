//META: generated=true
//META: model=gpt-6.1-sol
//META: generated_at=2026-10-13T10:55:00Z
//META: source_commit=32438d70ef57a4f2bb4a98c8a1e3b7fcc9f47d90
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#selected-receiver-validation
//META: doc_section_digest=e191be62b763f385e5f5aa6d3c8653f6a07af0fb4a17d527810b78117642f7bb
//META: purpose=After CompleteOverloadCandidate returns the selected call, SelectedExpressionFinalizer validates its callable receiver and rejects a mutating method on an immutable base.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Overload resolution selects `bump` without using receiver mutability as a candidate filter.
// After `CompleteOverloadCandidate` returns the selected call,
// `SelectedExpressionFinalizer::visitInvokeExpr` finalizes its callable operand.
// `validateCallable` then calls `validateCallableReceiver`. Because `look` is non-mutating, its
// reconstructed `this` is immutable, and that helper emits the diagnostic below.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

struct S
{
    int v;
    [mutating] int bump() { v = v + 1; return v; }
    int look() { return bump(); }
//CHECK:                    ^ mutating method 'bump' cannot be called on an immutable value
}

void main()
{
    S s;
    s.v = 1;
    printf("%d\n", s.look());
}
