//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:43+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/generics.md#parameter-binding
//META: doc_section_digest=7b3e1c2f4d8a0e9f5b2c6d1e3f7a4b8c0d2e6f9a1b5c3e7f0d4a8b2c6e0f4a
//META: purpose=When inference is ambiguous between fundamental scalar types, the element type with the highest promotion rank is chosen; mixing int8_t and int32_t resolves to int32_t.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc states: when inference is ambiguous and all inferred types are
// fundamental scalar types, the element type with the highest promotion rank is used.
// Promotion ranks: int8_t < ... < int32_t < ... < float < double.
// Passing int8_t and int32_t to a generic `f(T, T)` should resolve T to int32_t.

//TEST:INTERPRET(filecheck=CHECK):

func addInts<T : IArithmetic>(T a, T b) -> T { return a + b; }

void main()
{
    int8_t  small = int8_t(3);
    int32_t big   = int32_t(100);
    // int8_t and int32_t — T resolves to int32_t (higher rank). Result: 103.
    int32_t result = addInts(small, big);
    // CHECK: 103
    printf("%d\n", result);
}
