//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/types.md#nodes
//META: doc_section_digest=c7f55cb42aaa9e03df513fb2e3ff5bbcf380f2c09432d750a2aa9c55dfc56801
//META: purpose=An OptionalType<T> carries a value type and holds either a wrapped T or no value; `.hasValue` and `.value` expose the two states.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies both states of the optional type: one Optional<int> is
// initialised with a wrapped value and one with no value. CHECK asserts
// hasValue plus the unwrapped payload for the first, and hasValue false
// for the second, so a type that collapsed the two states would fail.

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
