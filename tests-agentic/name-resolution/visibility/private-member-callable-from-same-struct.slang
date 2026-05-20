//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=ed8f508fc647eecd788a4bd2bb63a4a6f5c80246
//META: doc_ref=docs/llm-generated/name-resolution/visibility.md#per-keyword-semantics
//META: doc_section_digest=ce44867411acd029099bae586609553fe57570c9431906d8d42a8d17c424b6bc
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
