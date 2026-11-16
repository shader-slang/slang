//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/modifiers.md#receiver-parameter-modes
//META: doc_section_digest=0a94769a669aa12d0fcebf6cfa246eb8793500358434d0e57afd65ad8545f45a
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
