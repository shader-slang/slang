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

// Verifies that ambiguous inference between scalar types resolves to the highest-ranked type.
// `addInts(int8_t(3), int32_t(100))` must infer T as int32_t (higher promotion rank than int8_t);
// CHECK asserts 103, which is only correct if the wider type was chosen and no truncation occurred.

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
