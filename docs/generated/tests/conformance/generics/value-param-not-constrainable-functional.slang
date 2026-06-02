//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:43+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/generics.md#description
//META: doc_section_digest=7b3e1c2f4d8a0e9f5b2c6d1e3f7a4b8c0d2e6f9a1b5c3e7f0d4a8b2c6e0f4a
//META: purpose=No assumptions are made about generic value parameters other than their declared type; a value parameter of type uint is treated as an integer constant.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a `let N : uint` value parameter carries its compile-time constant into the struct body.
// Two specializations `Sized<7>` and `Sized<100>` are instantiated and `capacity()` called on each;
// CHECK asserts 7 and 100, proving the value parameter is preserved through specialization.

//TEST:INTERPRET(filecheck=CHECK):

struct Sized<let N : uint>
{
    uint capacity() { return N; }
}

void main()
{
    Sized<7> s;
    // CHECK: 7
    printf("%u\n", s.capacity());
    Sized<100> t;
    // CHECK: 100
    printf("%u\n", t.capacity());
}
