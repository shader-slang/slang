//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:06+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/types-interface.md#description
//META: doc_section_digest=2ed61530bd334a97114c08a34024c5fb9ccf09620d7a60bd6a64f9d345223014
//META: purpose=An interface may inherit from another interface; inherited members add to the inheriting interface, and a concrete type conforming to the child interface must satisfy all inherited requirements.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc states "An interface may also inherit from another interface.
// The inherited members add to the inheriting interface."
// Verifies that a struct conforming to a child interface satisfies
// both child and inherited parent requirements.

//TEST:INTERPRET(filecheck=CHECK):

interface IBase
{
    int getA();
    static const int8_t bias;
}

interface IDerived : IBase
{
    int getB();
}

struct Impl : IDerived
{
    static const int8_t bias = 2;
    int _a;
    int _b;

    int getA() { return _a + bias; }
    int getB() { return _b; }
}

void main()
{
    Impl obj;
    obj._a = 10;
    obj._b = 20;
    //CHECK: a=12 b=20
    printf("a=%d b=%d\n", obj.getA(), obj.getB());
}
