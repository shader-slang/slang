//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:30:00+00:00
//META: source_commit=330c9a8d807b9f9352e4754f466d1244ae681cff
//META: doc_ref=docs/llm-generated/glossary.md#terms
//META: doc_section_digest=c590da2fa78de86b023f0aa1d0df2f46a79bb2de597705bc7ed2fee90af2869b
//META: purpose=shadowing: a name defined in an inner scope hides a same-named name from an outer scope for the duration of that inner scope.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the "shadowing" glossary entry: the inner `x` shadows
// the outer `x` so the printf prints the inner value (7), not the
// outer value (3).

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int x = 3;
    {
        int x = 7;
        // CHECK: inner=7
        printf("inner=%d\n", x);
    }
}
