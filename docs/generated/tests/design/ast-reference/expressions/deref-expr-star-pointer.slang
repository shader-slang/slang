//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#nodes
//META: doc_section_digest=0b0dfab9f13850a71e13b9ce6f8fb9a433ddb72267a526bf6454583638151a5b
//META: purpose=Verifies that a written `*p` dereferences a pointer value and reads the pointee, matching the `p->m` spelling on the same pointer.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Dereference has no dedicated surface node: `*p` goes through the unary
// prefix operator ladder and only becomes a dereference after checking. The
// same field is read twice on the same pointer, once as `(*p).v` and once as
// `p->v`; the CHECKs assert both spellings observe the same stored value.

//TEST:INTERPRET(filecheck=CHECK):

struct S { int v; }

void main()
{
    S s = { 42 };
    S* p = __getAddress(s);
    // CHECK: star=42
    printf("star=%d\n", (*p).v);
    // CHECK: arrow=42
    printf("arrow=%d\n", p->v);
}
