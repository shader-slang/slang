//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-06-01T16:00:00+00:00
//META: source_commit=d25453d7f0b4867db4cb5eabf34fb6cd088cf596
//META: doc_ref=docs/language-reference/types-array.md#declaration-syntax
//META: doc_section_digest=ad7e70e01944de764a4e5bc82497a352737e4133daa640a2bfae8511b826b4e5
//META: purpose=All three array-declaration forms (`var x : int[3]`, `int[3] x`, `int x[3]`) declare the same type and accept the same initializer; values read back identically.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    var a : int[3] = { 1, 2, 3 };
    int[3] b = { 1, 2, 3 };
    int c[3] = { 1, 2, 3 };
    bool eq = (a[0] == b[0]) && (b[0] == c[0])
           && (a[1] == b[1]) && (b[1] == c[1])
           && (a[2] == b[2]) && (b[2] == c[2]);
    //CHECK: a=1,2,3 b=1,2,3 c=1,2,3 eq=1
    printf("a=%d,%d,%d b=%d,%d,%d c=%d,%d,%d eq=%d\n",
           a[0], a[1], a[2], b[0], b[1], b[2], c[0], c[1], c[2],
           eq ? 1 : 0);
}
