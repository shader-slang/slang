//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:50:00+00:00
//META: source_commit=bbd84dc65e58598bfa71fafe72764b4076b0869b
//META: doc_ref=docs/llm-generated/ast-reference/modifiers.md#parameter-direction-and-storage-class-modifiers
//META: doc_section_digest=649151368fdbf5799c5b5465837a522d66af4e2a0ea43703031b86df41af5fcc
//META: purpose=`const` makes a local variable immutable; assigning to it is diagnosed.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `ConstModifier` marks the variable as a compile-time-immutable
// binding. The checker rejects any subsequent assignment with an
// l-value-of-type-const-is-not-modifiable diagnostic.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

void main()
{
    const int x = 7;
    x = 42;
    //CHECK: error
}
