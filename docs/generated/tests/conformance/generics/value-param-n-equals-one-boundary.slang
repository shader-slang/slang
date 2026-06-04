//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:43+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/generics.md#description
//META: doc_section_digest=7b3e1c2f4d8a0e9f5b2c6d1e3f7a4b8c0d2e6f9a1b5c3e7f0d4a8b2c6e0f4a
//META: purpose=A generic struct specialized with value parameter N=1 (boundary case) has a single-element array that can be read and written.
//META: intent=boundary
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary: N=1 is the smallest useful value for an array-sized value parameter.

//TEST:INTERPRET(filecheck=CHECK):

struct Arr<let N : uint>
{
    int data[N];
}

void main()
{
    Arr<1> a;
    a.data[0] = 99;
    // CHECK: 99
    printf("%d\n", a.data[0]);
}
