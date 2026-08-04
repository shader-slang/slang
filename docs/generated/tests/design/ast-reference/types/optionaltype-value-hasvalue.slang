//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ast-reference/types.md#aggregate-collection-types
//META: doc_section_digest=acec388be6a124a34086d58d95713bb5cd33eeac4b8fcf6534173cfc8ec11143
//META: purpose=An OptionalType<T> holds either none or a wrapped T; .hasValue and .value expose the two states.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies OptionalType is Optional<T> distinguishing wrapped-value
// from no-value. One optional is initialized to 42, another to none;
// CHECK asserts hasValue/value for the wrapped case and hasValue=0
// for the none case, covering both states.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    Optional<int> a = 42;
    Optional<int> b = none;
    // CHECK: a.hasValue=1
    printf("a.hasValue=%d\n", a.hasValue ? 1 : 0);
    // CHECK: a.value=42
    printf("a.value=%d\n", a.value);
    // CHECK: b.hasValue=0
    printf("b.hasValue=%d\n", b.hasValue ? 1 : 0);
}
