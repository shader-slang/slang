//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-06-01T15:30:00+00:00
//META: source_commit=d25453d7f0b4867db4cb5eabf34fb6cd088cf596
//META: doc_ref=docs/language-reference/types-vector-and-matrix.md#element-access
//META: doc_section_digest=b0ab4edd7e624713af836700667d410c7afa9b2f65ed30df6db9b64ee04184ba
//META: purpose=Swizzle assign writes the named columns; the doc example `v.xz = vector<int,2>(-1, -3)` results in `{ -1, 2, -3, 4 }`.
//META: intent=boundary
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int4 v = int4(1, 2, 3, 4);
    v.xz = int2(-1, -3);
    //CHECK: -1 2 -3 4
    printf("%d %d %d %d\n", v.x, v.y, v.z, v.w);
}
