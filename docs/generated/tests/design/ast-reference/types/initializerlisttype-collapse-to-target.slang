//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/types.md#nodes
//META: doc_section_digest=e9219cdb5c991479475f495fed529b3a717626417843075cd38373dfd41801fa
//META: purpose=An InitializerListType `{a, b, ...}` is a pseudo-type that collapses into the target type once matched, filling a struct field-wise and an array element-wise.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the same brace-list syntax takes its meaning from the
// target it is matched against: one list fills a two-field struct and
// another fills a three-element array. CHECK asserts both collapsed
// results, so a brace list that failed to adopt its target's shape
// would show up as a wrong field or element value.

//TEST:INTERPRET(filecheck=CHECK):

struct P { int x; int y; }

void main()
{
    P p = { 3, 4 };
    int a[3] = { 10, 20, 30 };
    // CHECK: p=3,4
    printf("p=%d,%d\n", p.x, p.y);
    // CHECK: a=10,20,30
    printf("a=%d,%d,%d\n", a[0], a[1], a[2]);
}
