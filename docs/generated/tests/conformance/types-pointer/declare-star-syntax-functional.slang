//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:32+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/types-pointer.md#syntax
//META: doc_section_digest=3eac3aff8a5d5daa072f3cad641be3c7b7a2f0e1ecf59515c5939a7a993d85ec
//META: purpose=The `T*` declaration syntax creates a pointer-to-T; address-of `&` and dereference `*` work round-trip.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `T*` declares a pointer-to-T and that `&` / `*` round-trip correctly.
// Assigns `int* p = &x`, then reads `*p`; CHECK asserts the value matches
// the original, proving declaration, address-of, and dereference all work together.
//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int x = 42;
    int* p = &x;
    //CHECK: 42
    printf("%d\n", *p);
}
