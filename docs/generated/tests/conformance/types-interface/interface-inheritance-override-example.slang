//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:06+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/types-interface.md#example
//META: doc_section_digest=2ed61530bd334a97114c08a34024c5fb9ccf09620d7a60bd6a64f9d345223014
//META: purpose=The doc's main Example section: ITest inherits IBase, ConcreteInt16 overrides a default, and a function accepting ITest-typed parameter works for both concrete types with correct results.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Exercise the doc's Example section: interface inheritance, default impl,
// override with the `override` modifier, and calling via an interface-typed
// function parameter (compile-time specialization).

//TEST:INTERPRET(filecheck=CHECK):

interface IBase
{
    int getA();
    static const int8_t bias;
}

interface ITest : IBase
{
    int someFunc()
    {
        return getA() + bias;
    }

    static int getUnderlyingWidth();
}

struct ConcreteInt32 : ITest
{
    static const int8_t bias = 3;
    int32_t a;

    int32_t getA() { return a; }
    static int getUnderlyingWidth() { return 32; }
}

struct ConcreteInt16 : ITest
{
    static const int8_t bias = 1;
    int16_t a;

    int getA() { return int(a); }

    override int someFunc() { return int(a) + 5; }
    static int getUnderlyingWidth() { return 16; }
}

int getValSquared(ITest i)
{
    return i.getA() * i.getA();
}

void main()
{
    ConcreteInt32 c32;
    c32.a = 4;
    ConcreteInt16 c16;
    c16.a = 3;

    // getValSquared specializes for each concrete type
    int sq32 = getValSquared(c32);
    int sq16 = getValSquared(c16);

    // someFunc: default impl for c32 = getA() + bias = 4 + 3 = 7
    int sf32 = c32.someFunc();
    // someFunc: overridden for c16 = a + 5 = 3 + 5 = 8
    int sf16 = c16.someFunc();

    int w32 = ConcreteInt32.getUnderlyingWidth();
    int w16 = ConcreteInt16.getUnderlyingWidth();

    //CHECK: sq32=16 sq16=9 sf32=7 sf16=8 w32=32 w16=16
    printf("sq32=%d sq16=%d sf32=%d sf16=%d w32=%d w16=%d\n",
           sq32, sq16, sf32, sf16, w32, w16);
}
