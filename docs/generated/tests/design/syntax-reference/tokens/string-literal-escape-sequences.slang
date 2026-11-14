//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/tokens.md#content-tokens
//META: doc_section_digest=c2efc22fd6b393971554625ec6e5d1342ed8b02f96b9afdf61ab48806b3d700e
//META: purpose=Boundary - escape sequences (`\t`, `\n`, `\\`, `\"`) inside a regular StringLiteral are accepted by the lexer and round-trip through to emitted C++.
//META: intent=boundary
//META: pipeline_stage=lex
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary probe: the four common escape sequences must lex inside
// a double-quoted string. The emitted CPP keeps the escapes literal
// (the lexer does not yet decode them — that happens later).

//TEST:SIMPLE(filecheck=CHECK):-target cpp -entry main -stage compute

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    printf("%s\n", "a\tb\nc\\d\"e");
    //CHECK: "a\tb\nc\\d\"e"
}
