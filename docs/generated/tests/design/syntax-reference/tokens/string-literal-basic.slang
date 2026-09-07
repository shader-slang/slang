//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/tokens.md#content-tokens
//META: doc_section_digest=98ff1b59e664e45b3e67fde56e5d2482e7d7845bb990f58cbca3a8c2edfcdd84
//META: purpose=A double-quoted `StringLiteral` is recognized by the lexer and its text content survives end-to-end through to emitted code.
//META: intent=functional
//META: pipeline_stage=lex
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies a plain double-quoted string literal is accepted by the
// lexer and emitted verbatim into CPP target output. The CHECK pattern
// looks for the literal text in the emitted CPP source.

//TEST:SIMPLE(filecheck=CHECK):-target cpp -entry main -stage compute

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    printf("%s\n", "hello-world");
    //CHECK: "hello-world"
}
