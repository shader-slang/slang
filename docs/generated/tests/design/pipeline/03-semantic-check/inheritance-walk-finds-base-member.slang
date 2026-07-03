//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/name-resolution/lookup.md#member-lookup
//META: doc_section_digest=b402a1bc67693a51de1de19d0938b42435b527692e0ae190ce1e55ebcdcd461c
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
