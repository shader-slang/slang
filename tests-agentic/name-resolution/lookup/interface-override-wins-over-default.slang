//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:48:38Z
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/llm-generated/name-resolution/lookup.md#interface-requirements-vs-default-implementations
//META: doc_section_digest=76f30c7b2e2657aa9a61baf14ba82d0fa96246f9396246481386a52c4f959681
//META: purpose=A conforming-type override of an interface requirement shadows the interface's default implementation; the override is what executes.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says: "A user-provided extension member can shadow an
// interface default implementation. ... a witness override on a
// conforming type wins over the default." Verify that the struct's
// override fires, not the interface default.

//TEST:INTERPRET(filecheck=CHECK):

interface IGreet
{
    int hello() { return 1; }   // default
}

struct A : IGreet
{
    override int hello() { return 99; }
}

int callIt<T : IGreet>(T t) { return t.hello(); }

void main()
{
    A a;
    // CHECK: g=99
    printf("g=%d\n", callIt(a));
}
