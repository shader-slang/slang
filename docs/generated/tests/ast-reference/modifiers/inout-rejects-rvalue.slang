//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:50:00+00:00
//META: source_commit=bbd84dc65e58598bfa71fafe72764b4076b0869b
//META: doc_ref=docs/generated/design/ast-reference/modifiers.md#parameter-direction-and-storage-class-modifiers
//META: doc_section_digest=649151368fdbf5799c5b5465837a522d66af4e2a0ea43703031b86df41af5fcc
//META: purpose=An `inout` parameter requires an l-value argument; passing a literal is diagnosed.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `InOutModifier` requires that the caller's argument is an l-value,
// because the callee writes back through it. Passing a literal (an
// r-value) is rejected by the checker.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void inc(inout int x)
{
    x = x + 1;
}

void main()
{
    inc(7);
    //CHECK: error
    //CHECK: E30047
}
