//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/visibility.md#per-keyword-semantics
//META: doc_section_digest=7d96ab9817d6f6251e2148b7d4d9959fb9ec73041354e919db537f46458d416e
//META: purpose=A `private` member is visible from another method of the same aggregate; access through `this` succeeds.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Per-keyword semantics: `private` is visible only inside the same
// aggregate. A public method on the same struct can still call a
// private one and read a private field.

//TEST:INTERPRET(filecheck=CHECK):

struct S
{
    private int secret;
    private int doubleSecret() { return secret * 2; }
    int observe() { return doubleSecret(); }

    __init(int x) { secret = x; }
}

void main()
{
    S s = S(7);
    // CHECK: observe=14
    printf("observe=%d\n", s.observe());
}
