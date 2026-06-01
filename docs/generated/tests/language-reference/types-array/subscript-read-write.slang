//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-06-01T16:00:00+00:00
//META: source_commit=d25453d7f0b4867db4cb5eabf34fb6cd088cf596
//META: doc_ref=docs/language-reference/types-array.md#declaration-syntax
//META: doc_section_digest=ad7e70e01944de764a4e5bc82497a352737e4133daa640a2bfae8511b826b4e5
//META: purpose=Basic subscript access: `arr[i]` reads, `arr[i] = v` writes; access is 0-indexed.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int arr[5] = { 10, 20, 30, 40, 50 };
    arr[2] = 99;
    //CHECK: 10 20 99 40 50
    printf("%d %d %d %d %d\n",
           arr[0], arr[1], arr[2], arr[3], arr[4]);
}
