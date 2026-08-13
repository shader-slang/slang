//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/base.md#syntaxnode-syntaxnodebase
//META: doc_section_digest=1ad4722af90d4681f39bfa8a4ce29ca156274cd102b43fbcd44252bd7eee3e2e
//META: purpose=Every parsed AST node carries a source location, so a check-time error on an expression is reported at that expression's own column.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the location captured when the expression was parsed
// survives into checking. The undefined identifier sits at column 13
// of its line; CHECK's caret span starts there rather than at the
// start of the declaration, which is only possible if the parsed node
// recorded its own location.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void main()
{
    int x = undefinedNameXyz;
//CHECK:    ^^^^^^^^^^^^^^^^ undefined identifier
//CHECK:    ^^^^^^^^^^^^^^^^ undefined identifier 'undefinedNameXyz'.
// CHECK: E30015
}
