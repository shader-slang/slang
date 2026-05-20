//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:30:00+00:00
//META: source_commit=330c9a8d807b9f9352e4754f466d1244ae681cff
//META: doc_ref=docs/llm-generated/glossary.md#terms
//META: doc_section_digest=c590da2fa78de86b023f0aa1d0df2f46a79bb2de597705bc7ed2fee90af2869b
//META: purpose=overload resolution: among candidates sharing a name, the one whose parameter type matches the argument without a conversion is preferred over the one requiring an implicit conversion.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the "overload resolution" glossary entry: passing a
// `float` argument selects the `float` overload (id=2) rather than
// the `int` overload (id=1) that would have required a conversion.

//TEST:INTERPRET(filecheck=CHECK):

int pick(int v) { return 1; }
int pick(float v) { return 2; }

void main()
{
    // CHECK: pick=2
    printf("pick=%d\n", pick(1.5f));
}
