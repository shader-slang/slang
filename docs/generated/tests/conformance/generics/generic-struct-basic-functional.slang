//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:43+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/generics.md#description
//META: doc_section_digest=7b3e1c2f4d8a0e9f5b2c6d1e3f7a4b8c0d2e6f9a1b5c3e7f0d4a8b2c6e0f4a
//META: purpose=A generic struct with a type parameter can be specialized with a concrete type and used as a normal struct.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Generic struct with a type parameter `T` is specialized to `Wrapper<int>` and `Wrapper<float>`.
// The specialized struct behaves as a concrete type.

//TEST:INTERPRET(filecheck=CHECK):

struct Wrapper<T>
{
    T value;
}

void main()
{
    Wrapper<int> wi;
    wi.value = 42;
    Wrapper<float> wf;
    wf.value = 3.14;
    //CHECK: 42
    printf("%d\n", wi.value);
}
