//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T21:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#nodes
//META: doc_section_digest=5cae24401ed06bd3567242934951f344da434f10804637bdd679d55ab6b5bb2b
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
