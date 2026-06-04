//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:32+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/types-pointer.md#generic-pointer
//META: doc_section_digest=0a3cd3e5c061a1c2465d71accdb421b9abe6d03e1a1b0bfbd0b67db662ed7071
//META: purpose=`Ptr<T>` generic pointer type can be declared, assigned from `&obj`, and dereferenced; yields the same value as a `T*` pointer.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the `Ptr<T>` generic type alias is usable as a pointer.
// Declares `Ptr<int> p = &x` and dereferences it; CHECK asserts the
// value read equals the original, confirming the alias behaves like `T*`.
//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int x = 55;
    Ptr<int> p = &x;
    //CHECK: 55
    printf("%d\n", *p);
}
