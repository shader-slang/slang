//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/statements.md#loop-family
//META: doc_section_digest=96f5a93c2d3b7b0c3cfafeba9cc0e6087ddd11d34203934cd833e35b847de3f5
//META: purpose=A `do` statement's body is parsed before the compiler decides what was built: a following `while` makes a do-while loop and a following `catch` makes a protected body, and anything else is an error.
//META: intent=negative
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the `do` form is only completed by `while` or `catch`. A
// `do { }` with neither following it must be rejected; the CHECK pins
// the E20004 diagnostic, whose message names the two tokens that would
// have completed the statement.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target cpp -entry computeMain -stage compute

RWStructuredBuffer<int> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    int n = 0;
    do { n = n + 1; }
    outputBuffer[0] = n;
/*CHECK:
    ^^^^^^^^^^^^ unexpected token
    ^^^^^^^^^^^^ unexpected identifier, expected 'while' or 'catch'
*/
}
