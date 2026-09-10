//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#nodes
//META: doc_section_digest=0b0dfab9f13850a71e13b9ce6f8fb9a433ddb72267a526bf6454583638151a5b
//META: purpose=Below language version 2026 a comma inside parentheses is still the comma operator rather than a tuple construction, so the parenthesized pair evaluates to its right-hand operand.
//META: intent=boundary
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the lower side of the tuple language-version gate. Without the 2026
// standard selected, `(10, 20)` is not a tuple at all: the comma is the comma
// operator, so the whole parenthesized expression has type `int` and value 20,
// and it assigns to an `int` without complaint. The CHECK reads that value
// back. Its counterpart tuple-expr-2026-makes-parens-a-tuple.slang compiles
// the same text with `-std 2026` and shows the type becoming
// `Tuple<int, int>`.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int x = (10, 20);
    printf("x=%d\n", x);
}

//CHECK: x=20
