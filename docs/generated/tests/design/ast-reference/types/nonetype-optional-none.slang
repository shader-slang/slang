//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/types.md#nodes
//META: doc_section_digest=71d352a8c5034422ec1e8eb2b4a93b52395bd85cc8305cee8432fdeb2101642c
//META: purpose=The NoneType of the `none` literal is accepted at an Optional-typed target without an explicit cast, whatever the optional's value type.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `none` has its own type which converts to any optional
// target: the same literal initialises an Optional<float> and an
// Optional<int> with no cast written. CHECK asserts hasValue is false
// for both and true for a value-initialised optional, so the literal
// really produced the empty state rather than a default payload.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    Optional<float> a = none;
    Optional<int> b = none;
    Optional<float> c = 1.0f;
    // CHECK: a=0
    printf("a=%d\n", a.hasValue ? 1 : 0);
    // CHECK: b=0
    printf("b=%d\n", b.hasValue ? 1 : 0);
    // CHECK: c=1
    printf("c=%d\n", c.hasValue ? 1 : 0);
}
