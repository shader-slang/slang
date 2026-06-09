//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:50:00+00:00
//META: source_commit=bbd84dc65e58598bfa71fafe72764b4076b0869b
//META: doc_ref=docs/generated/design/ast-reference/modifiers.md#parameter-direction-and-storage-class-modifiers
//META: doc_section_digest=649151368fdbf5799c5b5465837a522d66af4e2a0ea43703031b86df41af5fcc
//META: purpose=An `out` parameter writes a value back to the caller's variable.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `OutModifier` is the parameter-direction modifier for `out`. The
// callee's assignment to the `out` parameter must be visible to the
// caller after the call returns.

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
