//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#error-recovery
//META: doc_section_digest=ecd97134efc1fb844f5467e390d9248d7c2f8f41cabf069e67d46c245cea2ef3
//META: purpose=Resynchronization skips a balanced bracketed or parenthesised region whole, so garbage nested inside `[...]` and `(...)` adds no extra diagnostics.
//META: intent=negative
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that skipping is done a balanced group at a time rather than token
// by token. The bad statement puts unrecognised punctuators both outside and
// inside a `[...]` group and a `(...)` group; a token-at-a-time skip would
// re-enter the groups and could stop on the inner `]` or `)`. Exhaustive mode
// means only the one annotated error may be emitted, and the recovery must
// still land on the `;` so the following declaration parses normally — which
// it does, since no diagnostic is reported for it.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void main()
{
    @ [ @ @ @ ] ( @ @ ) ;
/*CHECK:
    ^ error E20002
    ^ syntax error.
*/
    int ok = 1;
}
