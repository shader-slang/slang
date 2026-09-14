//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#expressions
//META: doc_section_digest=4e9de34fbc8a8877c6b73c6110db4d1ebef2d0ddd10da2bdfd2742413c1d6015
//META: purpose=`,` is the loosest expression operator (level 15) inside a full `Expr`, but terminates the operand inside an `ArgList`.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the two comma contexts the ladder distinguishes. In
// `(a + 1, b + 5)` a full Expr is parsed, so the comma is the operator
// and the value is its right operand, 7. In `two(a + 1, b + 5)` each
// argument stops before the comma, so the call receives two operands
// and returns 2*100 + 7 = 207. A single reading of `,` cannot produce
// both numbers, which is what makes this observation decisive.

//TEST:INTERPRET(filecheck=CHECK):

int two(int x, int y) { return x * 100 + y; }

void main()
{
    int a = 1;
    int b = 2;
    int viaCommaOperator = (a + 1, b + 5);
    // CHECK: comma=7 call=207
    printf("comma=%d call=%d\n", viaCommaOperator, two(a + 1, b + 5));
}
