//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:30:00+00:00
//META: source_commit=330c9a8d807b9f9352e4754f466d1244ae681cff
//META: doc_ref=docs/llm-generated/glossary.md#terms
//META: doc_section_digest=c590da2fa78de86b023f0aa1d0df2f46a79bb2de597705bc7ed2fee90af2869b
//META: purpose=type inference: an omitted type on a `var` initialized from an int literal is inferred to `int`; overload resolution then picks the int overload.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the "type inference" glossary entry: when `var x = 42;`
// has no annotated type, the checker infers the type of `x` from
// the initializer. We probe the inferred type by calling an
// overloaded `kind` and observing which overload is chosen.

//TEST:INTERPRET(filecheck=CHECK):

int kind(int v) { return 1; }
int kind(float v) { return 2; }

void main()
{
    var x = 42;
    // CHECK: kind=1
    printf("kind=%d\n", kind(x));
}
