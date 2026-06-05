//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/generated/design/ast-reference/statements.md#requirecapabilitystmt
//META: doc_section_digest=07606d141f870c1d9f8f16631fe40ff51877bcadd709a1b267ba3b0b350e73d2
//META: purpose=A RequireCapabilityStmt is a statement-level capability assertion (`__requireCapability(...)`) scoped to the enclosing function; the function compiles when the target capability set satisfies the listed atoms.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says RequireCapabilityStmt "asserts that the surrounding
// function requires the listed capability atoms". This is the
// in-function-body form; the module-scope `RequireCapabilityDecl`
// form is covered by the declarations bundle. We compile under the
// HLSL target with the `hlsl` capability listed: a target without
// that capability would diagnose, but our test target does have it.

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
