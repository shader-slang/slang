//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:50:00+00:00
//META: source_commit=bbd84dc65e58598bfa71fafe72764b4076b0869b
//META: doc_ref=docs/generated/design/ast-reference/modifiers.md#mutability-autodiff-annotations
//META: doc_section_digest=017266f7b5d727836de4cd07bb1c2bfcdb0d82e30647d5be27f19fbfcea1a2dc
//META: purpose=A non-`[mutating]` struct method cannot assign through `this`; the checker diagnoses it.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Without `MutatingAttribute`, a struct method's `this` is immutable.
// Assigning to a field of `this` is rejected by the checker.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

struct S
{
    int n;

    void bad()
    {
        n = 42;
        //CHECK: error
        //CHECK: E30011
    }
}

void main()
{
    S s;
    s.n = 0;
    s.bad();
}
