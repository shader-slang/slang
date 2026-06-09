//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:32+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/types-pointer.md#syntax
//META: doc_section_digest=7d0d8d99e81aaca2b6f41962be8f0bffe859a83dddeba6909e61b74e024ba2ff
//META: purpose=Subtracting an integer `n` from a pointer-to-array-element yields a pointer `n` elements back; the resulting pointer dereferences to the correct element.
//META: intent=boundary
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that subtracting an integer from a pointer retreats by that many elements.
// Points to the last element of a 5-element array, subtracts 3, dereferences;
// CHECK asserts the value at the retreated position, proving subtraction is element-wise.
//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int arr[5] = { 5, 10, 15, 20, 25 };
    int* p = &arr[4];
    p = p - 3;
    //CHECK: 10
    printf("%d\n", *p);
}
