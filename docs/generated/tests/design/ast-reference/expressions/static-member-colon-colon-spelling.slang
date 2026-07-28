//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:01:06Z
//META: source_commit=8ac9e49a5b7020af6a8040544b9f76f0fbf40898
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#memberexpr-staticmemberexpr-derefmemberexpr
//META: doc_section_digest=58d3dc1b167dae22620293ef4a1c128bfcf79423ded9a56ab1bbc40bc734de7e
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
