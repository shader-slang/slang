//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ast-reference/types.md#nodes
//META: doc_section_digest=0acf92c990c51e23e6b949538b0504562447e6a609a849cd20f331b1ba8f82e3
//META: purpose=An InitializerListType `{a, b, ...}` collapses into the target type once matched; a brace list fills a struct field-wise and an array element-wise.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies InitializerListType's observable claim: a `{...}` list
// carries the initializer-list pseudo-type until matched, then lowers
// into the target. One brace list fills a struct field-wise, another
// fills an array element-wise. CHECK asserts both collapsed results.

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
