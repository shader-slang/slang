//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:08+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/types-struct.md#function-call-op
//META: doc_section_digest=25db753c000949cdb0161bb6d155f6566474fe2e1005a820a94f2bb0487efed3
//META: purpose=A `operator ()` declaration allows applying parameters to an object as if it were a function; multiple overloads are resolved by parameter count/type.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `operator ()` overloads with different arities are all reachable and that
// overload resolution selects the right one by argument count. Three calls m(), m(x), m(x,y)
// exercise the zero-, one-, and two-parameter overloads; CHECK asserts each returns the
// expected product of `base` with the supplied arguments.

//TEST:INTERPRET(filecheck=CHECK):

struct Multiplier
{
    float base;

    float operator () ()
    {
        return base;
    }

    float operator () (float x)
    {
        return base * x;
    }

    float operator () (float x, float y)
    {
        return base * x * y;
    }
}

void main()
{
    Multiplier m = { 2.0f };

    float r0 = m();
    float r1 = m(3.0f);
    float r2 = m(3.0f, 4.0f);

    // CHECK: 2
    printf("%.0f\n", r0);
    // CHECK: 6
    printf("%.0f\n", r1);
    // CHECK: 24
    printf("%.0f\n", r2);
}
