//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#error-recovery
//META: doc_section_digest=490a9dc1fa74d15b5c32d617920911d5d9b67dbf1c0dd0c9637afde42b648b85
//META: purpose=Recovery inside a block stops in front of the closing `}` rather than consuming it, so the next declaration parses and the partial tree still reaches semantic checking.
//META: intent=negative
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies two coupled guarantees. `bad` has no `;` after its garbage, so the
// only way out is the recover-before `}` rule; if the `}` were consumed instead
// the parser would swallow the end of `bad` and mis-parse `alsoBad` as nested,
// producing extra syntax errors. Exhaustive mode rejects any such extra
// diagnostic. The second function then contributes an undefined-identifier
// error, which is a checking-stage diagnostic — it can only appear if the
// best-effort AST built after the syntax error was still handed to the checker.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void bad()
{
    @ @ @
/*CHECK:
    ^ error E20002
    ^ syntax error.
*/
}

void alsoBad()
{
    int x = undefinedName;
//CHECK:    ^^^^^^^^^^^^^ error E30015
//CHECK:    ^^^^^^^^^^^^^ undefined identifier 'undefinedName'.
}

void main()
{
}
