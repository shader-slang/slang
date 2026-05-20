//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T22:00:00+00:00
//META: source_commit=331a01d9d0cc721d8fc19a46fec17a4a275ba0e0
//META: doc_ref=docs/llm-generated/ast-reference/types.md#general-purpose-types
//META: doc_section_digest=d1ff0a0974c79b15cca66d02e10faaa455cbee26ce299de54d386fa9d4a40656
//META: purpose=InitializerListType is the pseudo-type of a brace-initializer before it has been matched; the same `{a, b}` collapses into different concrete types depending on the target.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc names `InitializerListType` as "the pseudo-type of an
// initializer list before it has been matched to a target type".
// The user-facing claim: a brace-initializer `{a, b, c}` is matched
// to either a struct or an array depending on the target type, and
// the values land at the right slots in each case.

//TEST:INTERPRET(filecheck=CHECK):

struct Point { int x; int y; }

void main()
{
    Point p = { 7, 8 };
    int a[3] = { 1, 2, 3 };
    // CHECK: p.x=7
    printf("p.x=%d\n", p.x);
    // CHECK: p.y=8
    printf("p.y=%d\n", p.y);
    // CHECK: a=1,2,3
    printf("a=%d,%d,%d\n", a[0], a[1], a[2]);
}
