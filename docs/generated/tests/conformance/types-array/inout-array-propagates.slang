//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-06-01T16:00:00+00:00
//META: source_commit=d25453d7f0b4867db4cb5eabf34fb6cd088cf596
//META: doc_ref=docs/language-reference/types-array.md#declaration-syntax
//META: doc_section_digest=ad7e70e01944de764a4e5bc82497a352737e4133daa640a2bfae8511b826b4e5
//META: purpose=Companion to the pass-by-value test: when the parameter is declared `inout`, mutations DO propagate back to the caller.
//META: intent=boundary
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Pass-by-value is the default. To explicitly propagate
// mutations, the parameter must be declared `inout`. This test
// is the positive control to the `pass-by-value-not-decay`
// negative test in this same bundle.

//TEST:INTERPRET(filecheck=CHECK):

void mutateInOut(inout int x[3])
{
    x[0] = 99;
    x[1] = 88;
    x[2] = 77;
}

void main()
{
    int caller[3] = { 1, 2, 3 };
    mutateInOut(caller);
    //CHECK: caller=99 88 77
    printf("caller=%d %d %d\n", caller[0], caller[1], caller[2]);
}
