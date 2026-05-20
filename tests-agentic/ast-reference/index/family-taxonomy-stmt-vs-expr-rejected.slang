//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:58:41Z
//META: source_commit=30ae111120515b7406aa6f427a4eaaa28a0903d8
//META: doc_ref=docs/llm-generated/ast-reference/index.md#family-taxonomy
//META: doc_section_digest=ee941d37430adef45b384cb8db792e24ac11919541c2f1279e411361abc290a4
//META: purpose=The AST family taxonomy is real: a Stmt root and an Expr root are distinct, so a bare break statement outside a loop or switch is diagnosed by the checker (a Stmt-shape constraint, not an Expr-shape one).
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The index doc's #family-taxonomy mermaid diagram partitions the
// concrete AST surface across Stmt, Expr, Decl, Type, Modifier and
// Val roots. A cross-cutting consequence of this partition is that
// the checker validates a node according to its family root, not
// merely as syntax. A break statement is a Stmt-family leaf whose
// validity is family-specific: it requires an enclosing loop or
// switch context. Issuing one outside that context yields a
// family-specific diagnostic.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

void main()
{
    break;
//CHECK: 'break' must appear inside loop or switch constructs
}
