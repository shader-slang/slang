//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:50:00+00:00
//META: source_commit=bbd84dc65e58598bfa71fafe72764b4076b0869b
//META: doc_ref=docs/generated/design/ast-reference/modifiers.md#parameter-direction-and-storage-class-modifiers
//META: doc_section_digest=649151368fdbf5799c5b5465837a522d66af4e2a0ea43703031b86df41af5fcc
//META: purpose=An `inout` parameter both reads the caller's value and writes back to it.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `InOutModifier` refines `OutModifier`: an `inout` argument behaves
// as both an input (the callee sees the caller's value) and an output
// (assignment propagates back).

//TEST:INTERPRET(filecheck=CHECK):

void doubleIt(inout int x)
{
    x = x * 2;
}

void main()
{
    int v = 21;
    doubleIt(v);
    //CHECK: v=42
    printf("v=%d\n", v);
}
