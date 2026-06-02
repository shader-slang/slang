//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:43+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/generics.md#type-checking
//META: doc_section_digest=7b3e1c2f4d8a0e9f5b2c6d1e3f7a4b8c0d2e6f9a1b5c3e7f0d4a8b2c6e0f4a
//META: purpose=Type checking is performed before specialization: operations on a constrained generic type T only those allowed by the constraint are legal, and the correct operations are callable.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc states: type checking of parameterized types is performed based on their type
// constraints before specialization. An operation on a parameterized generic type is legal
// if it is legal for all possible concrete types conforming to the declared constraints.

//TEST:INTERPRET(filecheck=CHECK):

interface IScalable
{
    int scale(int factor);
}

struct ScalableInt : IScalable
{
    int _val;
    int scale(int factor) { return _val * factor; }
}

func scaleBy10<T : IScalable>(T x) -> int
{
    return x.scale(10);
}

void main()
{
    ScalableInt si;
    si._val = 7;
    // CHECK: 70
    printf("%d\n", scaleBy10(si));
}
