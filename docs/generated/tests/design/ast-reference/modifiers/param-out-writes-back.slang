//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/modifiers.md#parameter-direction-and-storage-class-modifiers
//META: doc_section_digest=96a720a22bcbf95fd50a791728387696a93f2171d6c726d180d1b7d56de54df4
//META: purpose=An `out` parameter writes a value back to the caller's variable.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `out` names the copy-out direction. `produce` never reads its
// parameter, only assigns 42 to it; CHECK asserts the caller's `x`, initialised
// to 0, reads back as 42 after the call, which can only happen if the assignment
// propagated out.

//TEST:INTERPRET(filecheck=CHECK):

void produce(out int dst)
{
    dst = 42;
}

void main()
{
    int x = 0;
    produce(x);
    //CHECK: x=42
    printf("x=%d\n", x);
}
