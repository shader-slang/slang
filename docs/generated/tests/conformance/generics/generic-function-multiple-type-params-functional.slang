//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:43+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/generics.md#description
//META: doc_section_digest=7b3e1c2f4d8a0e9f5b2c6d1e3f7a4b8c0d2e6f9a1b5c3e7f0d4a8b2c6e0f4a
//META: purpose=A generic function with multiple independent type parameters (T, U) can infer each from its respective argument; both are independently specialized.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Multiple independent type parameters: T and U are each inferred from their
// respective call-site argument.

//TEST:INTERPRET(filecheck=CHECK):

func concatInts<T : IInteger, U : IInteger>(T a, U b) -> int64_t
{
    return int64_t(a.toInt64()) * 1000 + int64_t(b.toInt64());
}

void main()
{
    int8_t  a = int8_t(5);
    int32_t b = int32_t(7);
    int64_t result = concatInts(a, b);
    // CHECK: 5007
    printf("%lld\n", result);
}
