//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:09:57Z
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#aggtypedecl-structdecl-classdecl-enumdecl
//META: doc_section_digest=a5782ff90acc1f9bd796853a7eeead0f64a6539af34d0a231454f705284208e7
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
