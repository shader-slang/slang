//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:43+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/generics.md#description
//META: doc_section_digest=7b3e1c2f4d8a0e9f5b2c6d1e3f7a4b8c0d2e6f9a1b5c3e7f0d4a8b2c6e0f4a
//META: purpose=A type constraint `T : IArithmetic` allows arithmetic operations on the type parameter; the function works for int and float.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// From the doc Examples section (Type constraint).
// `add2` uses `where T : IArithmetic` to allow `a + b` on the type parameter.
// Both int and float specializations work.

//TEST:INTERPRET(filecheck=CHECK):

func add2<T>(T a, T b) -> T where T : IArithmetic
{
    return a + b;
}

void main()
{
    int   ri = add2(3, 4);
    float rf = add2(1.5, 2.5);
    // CHECK: 7
    printf("%d\n", ri);
    // CHECK: 4
    printf("%d\n", (int)rf);
}
