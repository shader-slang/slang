//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/statements.md#requirecapabilitystmt
//META: doc_section_digest=a8400139e8c7d9141f8c731b8b1db45726011886792f3c2369114e05cb6ab67f
//META: purpose=A RequireCapabilityStmt is a statement-level capability assertion (`__requireCapability(...)`) scoped to the enclosing function; the function compiles when the target capability set satisfies the listed atoms.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the statement spelling of `__requireCapability` is accepted
// inside a function body and resolves the named atom against the
// compilation target. Compiling for HLSL while requiring `hlsl` must
// succeed; the CHECK asserts the enclosing code was emitted.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry computeMain -stage compute

void helper()
{
    __requireCapability(hlsl);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    helper();
    //CHECK: void
}
