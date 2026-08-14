//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#modifier-validation
//META: doc_section_digest=233c54717a586cdba7e515d0d6fbb4c7e6363f272d4b60a4aed8b9997a298ad9
//META: purpose=Modifier validation rejects calling a `[mutating]` method on an immutable receiver (E30050).
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the `[mutating]` modifier participates in call checking rather
// than being decorative. The receiver is a `let` binding, so it has no
// writable storage and the mutating method cannot legally run on it. CHECK
// pins E30050 at the call.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

struct S
{
    int x;
    [mutating]
    void bump()
    {
        x = x + 1;
    }
}

void main()
{
    let s = S(1);
    s.bump();
//CHECK: E30050
}
