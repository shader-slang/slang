//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:00:00+00:00
//META: source_commit=74db89b9f77cdced9c4d0c47f377b38fffb9180b
//META: doc_ref=docs/generated/design/name-resolution/lookup.md#member-lookup
//META: doc_section_digest=68a2d43cee4f2785028c9292caeaac660921efdc95b8ec2b5eface8a8b89a0dc
//META: purpose=Member lookup walks the inheritance chain so a method declared on an interface is callable through a generic constrained on that interface.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The lookup doc says member lookup delegates to the
// inheritance walk for DeclRefType targets. Verify a base-interface
// method is reachable on a conforming struct value through a
// generic with an interface constraint.

//TEST:INTERPRET(filecheck=CHECK):

interface IGreet
{
    int hello();
}

struct A : IGreet
{
    int hello() { return 7; }
}

int greet<T : IGreet>(T t) { return t.hello(); }

void main()
{
    A a;
    // CHECK: g=7
    printf("g=%d\n", greet(a));
}
