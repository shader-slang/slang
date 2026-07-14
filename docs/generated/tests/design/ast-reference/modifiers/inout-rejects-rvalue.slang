//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:50:00+00:00
//META: source_commit=bbd84dc65e58598bfa71fafe72764b4076b0869b
//META: doc_ref=docs/generated/design/ast-reference/modifiers.md#parameter-direction-and-storage-class-modifiers
//META: doc_section_digest=bd95695034d63fdcecbd599f22c84217f47af88c7597c20e6f353db58735f570
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
