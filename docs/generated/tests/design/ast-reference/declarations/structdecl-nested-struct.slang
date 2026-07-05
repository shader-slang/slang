//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#aggtypedecl-structdecl-classdecl-enumdecl
//META: doc_section_digest=60c3e2924d17c23b1f3bc412881a4c5e71f81efe23ecdd996e436f61429ac8c8
//META: purpose=A StructDecl can nest another StructDecl as a member; the nested type is reachable through dotted member access.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that AggTypeDecl's container behaviour extends to nesting:
// a struct can declare a struct field, and member access on the
// outer struct reaches the inner struct's field.

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
