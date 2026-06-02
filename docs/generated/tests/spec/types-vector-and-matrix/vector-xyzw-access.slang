//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-06-01T15:30:00+00:00
//META: source_commit=d25453d7f0b4867db4cb5eabf34fb6cd088cf596
//META: doc_ref=docs/language-reference/types-vector-and-matrix.md#element-access
//META: doc_section_digest=b0ab4edd7e624713af836700667d410c7afa9b2f65ed30df6db9b64ee04184ba
//META: purpose=Vector named-component access: `.x` is index 0, `.y` is 1, `.z` is 2, `.w` is 3.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int4 v = int4(7, 8, 9, 10);
    //CHECK: x=7 y=8 z=9 w=10
    printf("x=%d y=%d z=%d w=%d\n", v.x, v.y, v.z, v.w);
}
