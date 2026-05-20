//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:50:00+00:00
//META: source_commit=bbd84dc65e58598bfa71fafe72764b4076b0869b
//META: doc_ref=docs/llm-generated/ast-reference/modifiers.md#parameter-direction-and-storage-class-modifiers
//META: doc_section_digest=649151368fdbf5799c5b5465837a522d66af4e2a0ea43703031b86df41af5fcc
//META: purpose=An `in` parameter is a copy; mutation inside the callee does not propagate to the caller.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `InModifier` indicates the default parameter direction: the callee
// receives a copy of the value. Assigning the parameter inside the
// callee changes only the local copy.

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
