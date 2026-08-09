//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/statements.md#blockstmt-and-seqstmt
//META: doc_section_digest=286b56c30cbbe8b637fe3e2feda9744698b33bf0a2434728ce750ab093cc9caf
//META: purpose=A `switch` introduces no scope of its own and takes its lexical scope from the block the parser builds for its body, so a variable declared under one case label is in scope under the following case labels.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies name lookup treats the whole switch body as one scope. An
// outer `shared` is initialized, then a same-named declaration appears
// under `case 1:`, and `default:` reads `shared`. The CHECK asserts the
// compiler reports that read as uninitialized: that can only happen if
// the name under `default:` resolved to the case-1 declaration rather
// than to the initialized outer variable, which is exactly what a
// single body-wide scope means.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target cpp -entry computeMain -stage compute

RWStructuredBuffer<int> outputBuffer;

int classify(int v)
{
    int shared = 5;
    switch (v)
    {
    case 1:
        int shared = 10;
        return shared;
    default:
        return shared;
/*CHECK:
        ^^^^^^ use of uninitialized variable
        ^^^^^^ use of uninitialized variable 'shared'
*/
    }
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = classify(9);
}
