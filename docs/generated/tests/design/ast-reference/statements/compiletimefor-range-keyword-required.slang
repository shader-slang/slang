//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/statements.md#compiletimeforstmt
//META: doc_section_digest=b830cb9d7a4474794ded574f3685c7c52f984f5f5edfcb84dd550dbf0d9ef1ae
//META: purpose=`Range` is a required literal keyword in the compile-time `for` syntax, not an ordinary callable, so any other name in that position is rejected while the statement is parsed.
//META: intent=negative
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the bounds spelling is fixed syntax rather than a call that
// name lookup resolves. Substituting another identifier must fail at
// parse time; the CHECK pins the E20004 diagnostic, whose message names
// the expected `Range` token.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target cpp -entry computeMain -stage compute

RWStructuredBuffer<int> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    int sum = 0;
    $for(i in Span(4))
//CHECK:      ^^^^ unexpected token
//CHECK:      ^^^^ unexpected identifier, expected 'Range'
    {
        sum = sum + i;
    }
    outputBuffer[0] = sum;
}
