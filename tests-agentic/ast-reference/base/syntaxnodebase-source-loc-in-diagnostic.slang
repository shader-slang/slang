//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=ef4af96c0996402dfe65ab0fdd347e4ae7e1a742
//META: doc_ref=docs/llm-generated/ast-reference/base.md#syntaxnodebase-nodebase
//META: doc_section_digest=365446a2e5983bbdfefe5303b6d60ff1b71a4d35b7b5c29fba8b07848ac8cdbc
//META: purpose=Every SyntaxNodeBase carries a SourceLoc; the diagnostic for a check-time error points at the originating syntax node's column.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that an Expr (a SyntaxNode, transitively a SyntaxNodeBase)
// carries its source location through to the diagnostic. The
// undeclared identifier `undefinedNameXyz` triggers a diagnostic
// whose caret points back at that identifier's column.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void main()
{
    int x = undefinedNameXyz;
//CHECK:    ^^^^^^^^^^^^^^^^ undefined identifier
//CHECK:    ^^^^^^^^^^^^^^^^ undefined identifier 'undefinedNameXyz'.
}
