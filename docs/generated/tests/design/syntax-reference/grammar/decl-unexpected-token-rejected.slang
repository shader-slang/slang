//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#declarations
//META: doc_section_digest=6e6148eb1487c323e49c7334a54ff330766cb60411262b1281d59e4c5990c422
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
