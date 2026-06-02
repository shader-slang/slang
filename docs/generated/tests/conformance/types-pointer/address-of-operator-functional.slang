//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:32+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/types-pointer.md#syntax
//META: doc_section_digest=7d0d8d99e81aaca2b6f41962be8f0bffe859a83dddeba6909e61b74e024ba2ff
//META: purpose=The address-of operator `&` applied to an object yields a pointer whose dereference gives back the original value.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc states: "To obtain the address of an object, the address-of
// operator & is used." Verify that &obj produces a live pointer and
// *(&obj) == obj.
//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int a = 7;
    int* pa = &a;
    //CHECK: 7
    printf("%d\n", *pa);
}
