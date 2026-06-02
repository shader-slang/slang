//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:32+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/types-pointer.md#syntax
//META: doc_section_digest=3eac3aff8a5d5daa072f3cad641be3c7b7a2f0e1ecf59515c5939a7a993d85ec
//META: purpose=`T*` is equivalent to `Ptr<T, Access.ReadWrite, AddressSpace.Device>`; a value of type `T*` is assignable to the fully-qualified Ptr form and vice versa.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's Remark 1 states that `T*` is equivalent to
// Ptr<T, Access.ReadWrite, AddressSpace.Device>.
// Verify the equivalence is a true type alias: a `T*` value can be
// assigned to a `Ptr<T, Access.ReadWrite, AddressSpace.Device>` variable
// and both read the same value.
//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int x = 42;
    int* p1 = &x;
    Ptr<int, Access.ReadWrite, AddressSpace.Device> p2 = p1;
    //CHECK: 42 42
    printf("%d %d\n", *p1, *p2);
}
