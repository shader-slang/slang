//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#aggtypedecl-structdecl-classdecl-enumdecl
//META: doc_section_digest=ee81272d01f69d54c75b75fc6518d2ad8af9fdb4a6eed0176bdf0fe82ee19e5c
//META: purpose=A StructDecl can nest another StructDecl as a member; the nested type is reachable through dotted member access.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the shared container behaviour of aggregate declarations
// extends to nesting: a struct may declare a field of another struct type,
// and member access chains through it. The CHECK asserts a sum computed
// from both the nested field and a sibling field.

//TEST:INTERPRET(filecheck=CHECK):

struct Inner { int v; }
struct Outer { Inner inner; int extra; }

void main()
{
    Outer o;
    o.inner.v = 30;
    o.extra = 5;
    //CHECK: 35
    printf("%d\n", o.inner.v + o.extra);
}
