//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:30:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/ast-reference/expressions.md#nodes
//META: doc_section_digest=5cae24401ed06bd3567242934951f344da434f10804637bdd679d55ab6b5bb2b
//META: purpose=An InitializerListExpr `{a, b, c}` builds an aggregate value whose elements correspond positionally to the listed expressions.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// InitializerListExpr is `{ a, b, c }` per the doc. The elements
// must be assigned positionally to the aggregate's fields.

//TEST:INTERPRET(filecheck=CHECK):

struct Rect { int w; int h; int d; }

void main()
{
    Rect r = {7, 8, 9};
    // CHECK: w=7 h=8 d=9
    printf("w=%d h=%d d=%d\n", r.w, r.h, r.d);
}
