//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/modifiers.md#mutability--autodiff-annotations
//META: doc_section_digest=a162d3ea93755dfa0e856855015bcc93b11eb24c29ad4e2f7cda299e7c6be3b7
//META: purpose=A method without `[mutating]` cannot assign through `this`; the checker diagnoses it.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the default for a struct method is non-mutating, so `[mutating]` is
// load-bearing rather than optional decoration. The method assigns to a field
// with no attribute present and CHECK pins the not-modifiable error code -- the
// same code a write to a `const` produces.

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
