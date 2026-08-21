//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#memberexpr--staticmemberexpr--derefmemberexpr
//META: doc_section_digest=2e4c0cf09ff9c6fb2f3bd8e1c643d300367d45168a8b35d087ed154695482428
//META: purpose=The `::` surface form is a distinct static-member spelling: `Limits::kMax` reads a static member through the type without an instance.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the `::` member-access surface form, which the parser maps
// straight to a static member access (unlike `.`, which starts as a
// value member access). Reading a `static const int` via `Limits::kMax`
// must yield the declared constant; CHECK asserts the value, which is
// only reachable if the `::` spelling resolves the static member.

//TEST:INTERPRET(filecheck=CHECK):

struct Limits
{
    static const int kMax = 4321;
}

void main()
{
    int v = Limits::kMax;
    // CHECK: v=4321
    printf("v=%d\n", v);
}
