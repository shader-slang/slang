//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ast-reference/types.md#nodes
//META: doc_section_digest=0acf92c990c51e23e6b949538b0504562447e6a609a849cd20f331b1ba8f82e3
//META: purpose=The NoneType of `none` is accepted at any Optional target without an explicit cast; an Optional initialized to none reports hasValue false.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the NoneType singleton claim: `none` is accepted at an
// Optional-typed target with no explicit cast, producing the empty
// optional. CHECK asserts hasValue is false for a none-initialized
// Optional and true for a value-initialized one.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    Optional<float> a = none;
    Optional<float> b = 1.0f;
    // CHECK: a=0
    printf("a=%d\n", a.hasValue ? 1 : 0);
    // CHECK: b=1
    printf("b=%d\n", b.hasValue ? 1 : 0);
}
