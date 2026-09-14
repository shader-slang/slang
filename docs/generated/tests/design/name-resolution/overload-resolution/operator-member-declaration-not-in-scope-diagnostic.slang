//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#the-general-path
//META: doc_section_digest=fcdc0ef5f61233fa8b4fb6def9878dcee51b42c2d1bbde085f283c94b50f6fdd
//META: purpose=A user-declared operator+ written as a member of a struct is not a name the call site can see, so v1 + v2 reports no applicable overload.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the operator candidate set comes from ordinary scope-chain
// lookup of the operator name rather than from member lookup on the left
// operand. The only user declaration of operator+ here is a struct member, so
// no candidate for the struct operands exists; CHECK pins the no-overload
// error naming the operand types.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

struct V { int x; V operator+(V o) { V r; r.x = x + o.x; return r; } }

void main()
{
    V a; a.x = 1;
    V b; b.x = 2;
    V c = a + b;
//CHECK:    ^ no overload for '+' applicable to arguments of type (V, V)
    printf("tag=%d\n", c.x);
}
