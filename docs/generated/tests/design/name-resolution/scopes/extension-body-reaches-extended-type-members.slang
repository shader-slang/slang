//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/scopes.md#implicit-scopes
//META: doc_section_digest=19ad073ab979d4a331ac1f5fc84f58af039847fad8e90dbe9db085b55e74e2b0
//META: purpose=The extended type's members are not on the extension's scope chain but are still reached from an extension method through member lookup on the receiver.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies how an extension body reaches the members of the type it
// extends. Two methods in `extension Point` read the field `x`: one spells
// the receiver explicitly, the other writes the bare name and relies on the
// same member lookup being applied to the implicit receiver. CHECK asserts
// both compute from the stored 4 (8 and 12), showing the field is reachable
// through member lookup rather than through the extension's own scope chain.

//TEST:INTERPRET(filecheck=CHECK):

struct Point
{
    int x;
}

extension Point
{
    int viaReceiver() { return this.x * 2; }
    int viaImplicitReceiver() { return x * 3; }
}

void main()
{
    Point p;
    p.x = 4;
    // CHECK: a=8
    printf("a=%d\n", p.viaReceiver());
    // CHECK: b=12
    printf("b=%d\n", p.viaImplicitReceiver());
}
