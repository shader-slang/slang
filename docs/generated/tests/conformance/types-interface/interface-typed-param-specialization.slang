//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:06+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/types-interface.md#description
//META: doc_section_digest=2ed61530bd334a97114c08a34024c5fb9ccf09620d7a60bd6a64f9d345223014
//META: purpose=A function with an interface-typed parameter that is called with a compile-time-known concrete type is specialized for that concrete type; both concrete types produce correct results via the same function.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a function with an interface-typed parameter is correctly
// specialized for two distinct concrete types at their call sites.
// getValSquared is called with ConcreteInt32(a=4) and ConcreteInt16(a=3);
// CHECK asserts sq32=16 and sq16=9, confirming each specialization computes independently.

//TEST:INTERPRET(filecheck=CHECK):

interface IValue
{
    int getA();
}

struct ConcreteInt32 : IValue
{
    int32_t a;
    int32_t getA() { return a; }
}

struct ConcreteInt16 : IValue
{
    int16_t a;
    int getA() { return int(a); }
}

int getValSquared(IValue i)
{
    return i.getA() * i.getA();
}

void main()
{
    ConcreteInt32 c32;
    c32.a = 4;
    ConcreteInt16 c16;
    c16.a = 3;
    //CHECK: sq32=16 sq16=9
    printf("sq32=%d sq16=%d\n", getValSquared(c32), getValSquared(c16));
}
