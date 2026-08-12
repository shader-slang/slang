//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-06-01T16:00:00+00:00
//META: source_commit=d25453d7f0b4867db4cb5eabf34fb6cd088cf596
//META: doc_ref=docs/language-reference/types-array.md#element-count-inference-for-unknown-length-array
//META: doc_section_digest=ad7e70e01944de764a4e5bc82497a352737e4133daa640a2bfae8511b826b4e5
//META: purpose=Element-count inference: `int a[] = {1, 2, 3, 4}` infers length 4 and is equivalent to `int a[4] = {...}`.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int a[] = { 0xA, 0xB, 0xC, 0xD };
    int b[4] = { 0xA, 0xB, 0xC, 0xD };
    bool eq = (a[0] == b[0]) && (a[1] == b[1])
           && (a[2] == b[2]) && (a[3] == b[3]);
    //CHECK: a[0..3]=10 11 12 13 eq=1
    printf("a[0..3]=%d %d %d %d eq=%d\n",
           a[0], a[1], a[2], a[3], eq ? 1 : 0);
}
