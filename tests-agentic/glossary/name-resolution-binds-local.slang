//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:30:00+00:00
//META: source_commit=330c9a8d807b9f9352e4754f466d1244ae681cff
//META: doc_ref=docs/llm-generated/glossary.md#terms
//META: doc_section_digest=c590da2fa78de86b023f0aa1d0df2f46a79bb2de597705bc7ed2fee90af2869b
//META: purpose=name resolution: identifier uses are bound to their declarations taking scope into account; a use of `n` inside a function body refers to the function's parameter, and an outer free identifier resolves to the corresponding declaration at module scope.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the "name resolution" glossary entry: the use of `n`
// inside `square` binds to the parameter, and the use of `BIAS`
// binds to the module-scope constant. Both bindings must resolve
// or the test would fail to compile.

//TEST:INTERPRET(filecheck=CHECK):

static const int BIAS = 100;

int square(int n)
{
    return n * n + BIAS;
}

void main()
{
    // CHECK: r=109
    printf("r=%d\n", square(3));
}
