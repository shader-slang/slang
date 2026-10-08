//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#nodes
//META: doc_section_digest=0b0dfab9f13850a71e13b9ce6f8fb9a433ddb72267a526bf6454583638151a5b
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
