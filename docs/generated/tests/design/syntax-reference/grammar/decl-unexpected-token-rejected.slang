//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:03:32+00:00
//META: source_commit=ed8f508fc647eecd788a4bd2bb63a4a6f5c80246
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#declarations
//META: doc_section_digest=a694b52dfeecd881608de80ca6201bf1fab683a7794ec1d5453373d8559abf3b
//META: purpose=A token that cannot begin a `Decl` triggers a parse-stage diagnostic.
//META: intent=negative
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the parser rejects an ill-formed top-level construct.
// `@` is not a valid start-of-decl token, so the parser emits a
// diagnostic when it encounters it.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

@ int x = 1;
/*CHECK:
^ unexpected token
^ unexpected '@', expected identifier
      ^ unexpected token
      ^ unexpected identifier, expected ';'
        ^ unexpected token
        ^ unexpected '=', expected identifier
*/

void main()
{
}
