//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#nodes
//META: doc_section_digest=26c797434ab51700cf84e595593cca11917ffceaff2d95c7a0504524e997316e
//META: purpose=Verifies that an empty InitializerListExpr `{}` zero-initializes the aggregate — every field reads back as 0.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Lower-edge boundary for InitializerListExpr.args: the empty list.
// `Rect r = {};` must zero-initialize every field.

//TEST:INTERPRET(filecheck=CHECK):

struct Rect { int a; int b; int c; }

void main()
{
    Rect r = {};
    // CHECK: a=0 b=0 c=0
    printf("a=%d b=%d c=%d\n", r.a, r.b, r.c);
}
