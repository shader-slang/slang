//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=716ec597fc9c85111cd2fa06ba4e89bc4469b6b2
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#statements
//META: doc_section_digest=11f48bb43fb7900ef7dd3c2f4785da1cab7aaae9b90d2da44ecf5e8cf8e950b1
//META: purpose=Omitting the terminating semicolon of a ThrowStmt is a parse error.
//META: intent=negative
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The negative half of `ThrowStmt ::= 'throw' Expr ';'`. Its sibling
// stmt-throw-requires-semicolon.slang shows the well-formed form runs; this one
// pins the boundary itself, so that making the semicolon optional again would be
// caught rather than silently accepted. The caret lands on the `}` that follows
// the semicolon-less `throw`, which is where ParseThrowStatement's
// ReadToken(TokenType::Semicolon) reports.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl

enum TErr
{
    Boom
}

func trip(x : int) throws TErr -> int
{
    if (x > 0)
    {
        throw TErr.Boom
    }
/*CHECK:
    ^ E20001
    ^ unexpected '}', expected ';'
*/
    return 0;
}
