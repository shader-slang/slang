//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:30:00+00:00
//META: source_commit=330c9a8d807b9f9352e4754f466d1244ae681cff
//META: doc_ref=docs/llm-generated/glossary.md#terms
//META: doc_section_digest=c590da2fa78de86b023f0aa1d0df2f46a79bb2de597705bc7ed2fee90af2869b
//META: purpose=inlining: a call instruction is replaced with a copy of the callee's body, so the inlined computation produces the same observable result as the un-inlined form.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the "inlining" glossary entry: a small helper `add1`
// is inlined into `main`. The observable consequence is purely
// behavioural (Slang inlines aggressively but does not guarantee
// any specific textual emit), so we check the computed result.

//TEST:INTERPRET(filecheck=CHECK):

int add1(int v) { return v + 1; }

void main()
{
    int r = add1(add1(add1(40)));
    // CHECK: inl=43
    printf("inl=%d\n", r);
}
