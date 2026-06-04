//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:43+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/generics.md#description
//META: doc_section_digest=7b3e1c2f4d8a0e9f5b2c6d1e3f7a4b8c0d2e6f9a1b5c3e7f0d4a8b2c6e0f4a
//META: purpose=Generic value parameter declared with `let` syntax (`let N : uint`) can be used as a compile-time constant inside the struct.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A value parameter declared with `let N : uint` is usable as an array size.
// Two specializations with different N values produce independent types.

//TEST:INTERPRET(filecheck=CHECK):

struct FixedArray<T, let N : uint>
{
    T data[N];
}

void main()
{
    FixedArray<int, 2> a2;
    a2.data[0] = 10;
    a2.data[1] = 20;

    FixedArray<int, 4> a4;
    a4.data[0] = 1;
    a4.data[3] = 4;

    // CHECK: 30
    printf("%d\n", a2.data[0] + a2.data[1]);
    // CHECK: 5
    printf("%d\n", a4.data[0] + a4.data[3]);
}
