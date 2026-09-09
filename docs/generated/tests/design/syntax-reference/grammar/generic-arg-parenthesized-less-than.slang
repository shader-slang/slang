//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#-disambiguation
//META: doc_section_digest=dadb4ac2edfa61b92a02ca7923eeb5ee0f3e9aa8cc05a01e2bcfdef41f1a3055
//META: purpose=Inside a generic argument list `<` keeps its normal precedence, so a parenthesized `<` comparison is a valid generic argument.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the half of the in-generic-argument precedence rule that concerns
// `<`: unlike the closing-angle tokens, `<` is still an operator while the
// argument list is open, so a comparison can appear as a value argument. The
// generic's value parameter is `bool`, and the branch it selects is printed:
// 22 is only reachable if `(x < y)` was evaluated as the comparison 3 < 2.

//TEST:INTERPRET(filecheck=CHECK):

struct A<let B : bool>
{
    int v;
}

static const int x = 3;
static const int y = 2;

int pick<let B : bool>()
{
    return B ? 11 : 22;
}

void main()
{
    A<(x < y)> a;
    a.v = 5;
    // CHECK: r=22
    printf("r=%d\n", pick<(x < y)>());
    // CHECK: v=5
    printf("v=%d\n", a.v);
}
