//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:43+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/generics.md#description
//META: doc_section_digest=7b3e1c2f4d8a0e9f5b2c6d1e3f7a4b8c0d2e6f9a1b5c3e7f0d4a8b2c6e0f4a
//META: purpose=A generic function with a type parameter can be called with both int and float arguments; the type is inferred from the call site.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A generic function `identity<T>` can be specialized to `int` and `float`
// simply by calling it with values of those types. Type inference handles binding.

//TEST:INTERPRET(filecheck=CHECK):

T identity<T>(T x) { return x; }

void main()
{
    int   i = identity(99);
    float f = identity(1.5);
    // CHECK: 99
    printf("%d\n", i);
    // CHECK: 1
    printf("%d\n", (int)f);
}
