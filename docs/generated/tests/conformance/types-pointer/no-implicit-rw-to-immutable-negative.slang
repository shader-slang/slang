//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:32+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/types-pointer.md#traits
//META: doc_section_digest=e9381d81a059f23a446d37fc5fd347e5e21c4ae31ce71d3a8ae46d44bdd19142
//META: purpose=Assigning a `Ptr<T>` (read-write) value to an `ImmutablePtr<T>` variable without an explicit cast is rejected with E30019.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc states: "There is no implicit conversion from read-write to
// read-only pointers."
// Verify the compiler rejects the implicit assignment with E30019.
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target cpp

void main()
{
    int x = 10;
    Ptr<int> rw = &x;
    ImmutablePtr<int> ro = rw;
//CHECK: E30019
}
