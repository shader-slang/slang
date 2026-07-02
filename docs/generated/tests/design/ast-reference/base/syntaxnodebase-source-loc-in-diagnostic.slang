//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ast-reference/base.md#syntaxnode-syntaxnodebase
//META: doc_section_digest=2fec3eeec9abd5402c6ddba93ab2a194def801aba500168f43aaf444bfba60ac
//META: purpose=Every SyntaxNodeBase carries a SourceLoc; the diagnostic for a check-time error points at the originating syntax node's column.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that an Expr (a SyntaxNode, transitively a SyntaxNodeBase)
// carries its source location through to the diagnostic. The
// undeclared identifier `undefinedNameXyz` triggers a diagnostic
// whose caret points back at that identifier's column — only possible
// if the parser stored the loc on the SyntaxNodeBase for that node.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void main()
{
    int x = undefinedNameXyz;
//CHECK:    ^^^^^^^^^^^^^^^^ undefined identifier
//CHECK:    ^^^^^^^^^^^^^^^^ undefined identifier 'undefinedNameXyz'.
// CHECK: E30015
}
