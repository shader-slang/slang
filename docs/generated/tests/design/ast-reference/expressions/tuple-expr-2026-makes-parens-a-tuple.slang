//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#nodes
//META: doc_section_digest=0b0dfab9f13850a71e13b9ce6f8fb9a433ddb72267a526bf6454583638151a5b
//META: purpose=A parenthesized comma list is parsed as a tuple construction only at language version 2026 or later, where the same text that was a comma operator becomes a Tuple value.
//META: intent=boundary
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the upper side of the tuple language-version gate, using the type
// error as the observation. The initializer is the same text as in
// tuple-expr-comma-operator-below-2026.slang, where it has type `int`; with
// `-std 2026` selected the parser builds a tuple instead, so assigning it to
// an `int` is a type mismatch. CHECK pins E30019 together with the rendered
// type name, which is what identifies the parse as a tuple rather than any
// other change of meaning.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-std 2026

void main()
{
    int x = (10, 20);
//CHECK:    ^ E30019
//CHECK:    ^ expected an expression of type 'int', got 'Tuple<int, int>'
    printf("x=%d\n", x);
}
