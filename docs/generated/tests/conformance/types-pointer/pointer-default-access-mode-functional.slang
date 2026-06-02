//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:32+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/types-pointer.md#traits
//META: doc_section_digest=e9381d81a059f23a446d37fc5fd347e5e21c4ae31ce71d3a8ae46d44bdd19142
//META: purpose=The default pointer access mode is `Access.ReadWrite` and the default address space is `AddressSpace.Device`; `T*` and `Ptr<T, Access.ReadWrite, AddressSpace.Device>` are interchangeable.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc states: "The default pointer address space is AddressSpace.Device,
// and the default access mode is Access.ReadWrite."
// Verify that int* can be assigned to the fully-specified Ptr<> form,
// demonstrating they are the same type (both reads return the same value).
//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int x = 123;
    int* star       = &x;
    Ptr<int, Access.ReadWrite, AddressSpace.Device> full = star;
    //CHECK: 123 123
    printf("%d %d\n", *star, *full);
}
