//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/modifiers.md#parameter-direction-and-storage-class-modifiers
//META: doc_section_digest=96a720a22bcbf95fd50a791728387696a93f2171d6c726d180d1b7d56de54df4
//META: purpose=An `in` parameter is a copy; mutation inside the callee does not propagate to the caller.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `in` names the copy-in-only direction: the callee gets its
// own copy of the argument. `stomp` overwrites its `in` parameter with 999 and
// CHECK asserts the caller still observes 7, which is only true if the write
// stayed inside the callee's copy.

//TEST:INTERPRET(filecheck=CHECK):

void stomp(in int x)
{
    x = 999;
}

void main()
{
    int v = 7;
    stomp(v);
    //CHECK: v=7
    printf("v=%d\n", v);
}
