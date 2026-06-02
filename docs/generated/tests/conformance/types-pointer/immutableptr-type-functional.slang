//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:32+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/types-pointer.md#generic-pointer
//META: doc_section_digest=0a3cd3e5c061a1c2465d71accdb421b9abe6d03e1a1b0bfbd0b67db662ed7071
//META: purpose=`ImmutablePtr<T>` is a valid pointer alias; it can be declared via explicit cast from a `T*` and dereferenced to read the value.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `ImmutablePtr<T>` is a usable read-only pointer alias.
// Explicit-casts a `T*` to `ImmutablePtr<int>` and dereferences it;
// CHECK asserts the correct value is read back, proving the alias
// accepts the cast and supports dereference.
//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int x = 33;
    ImmutablePtr<int> ro = (ImmutablePtr<int>)(&x);
    //CHECK: 33
    printf("%d\n", *ro);
}
