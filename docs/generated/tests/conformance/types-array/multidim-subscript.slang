//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-06-01T16:00:00+00:00
//META: source_commit=d25453d7f0b4867db4cb5eabf34fb6cd088cf596
//META: doc_ref=docs/language-reference/types-array.md#declaration-syntax
//META: doc_section_digest=ad7e70e01944de764a4e5bc82497a352737e4133daa640a2bfae8511b826b4e5
//META: purpose=Multi-dimensional array element access: `arr[i][j]` indexes outer dimension then inner.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int m[2][3] = {
        { 11, 12, 13 },
        { 21, 22, 23 },
    };
    //CHECK: m[0][0]=11 m[1][2]=23 m[0][2]=13 m[1][0]=21
    printf("m[0][0]=%d m[1][2]=%d m[0][2]=%d m[1][0]=%d\n",
           m[0][0], m[1][2], m[0][2], m[1][0]);
}
