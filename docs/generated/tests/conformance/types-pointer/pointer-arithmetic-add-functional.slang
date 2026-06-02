//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:32+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/types-pointer.md#syntax
//META: doc_section_digest=7d0d8d99e81aaca2b6f41962be8f0bffe859a83dddeba6909e61b74e024ba2ff
//META: purpose=Adding an integer `n` to a pointer-to-array-element yields a pointer that points `n` elements forward; the resulting pointer dereferences to the correct element.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc states: "When a pointer points to an array element, an integer
// value may be added to or subtracted from it. The resulting pointer
// points to an element offset by that value."
// Verify that p + 2 advances by 2 elements.
//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int arr[5] = { 10, 20, 30, 40, 50 };
    int* p = &arr[0];
    p = p + 2;
    //CHECK: 30
    printf("%d\n", *p);
}
