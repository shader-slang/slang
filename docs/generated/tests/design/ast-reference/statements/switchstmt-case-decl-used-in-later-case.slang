//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/statements.md#blockstmt-and-seqstmt
//META: doc_section_digest=fcf4d2f955e68033a3f7d41a57eb38d3b947a94caa962232e1c2c51d654a34f2
//META: purpose=A `switch` introduces no scope of its own and takes its lexical scope from the block the parser builds for its body, so a variable declared under one case label is in scope under the following case labels.
//META: intent=regression
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the same single-scope body property end to end: `case 1:`
// declares `shared`, `case 2:` assigns and reads it, and the kernel is
// executed with 2. The CHECK asserts the buffer holds 20, so the
// declaration reached from the later case label was the same storage.
// The frontend accepts this program; running it exercises whether the
// backend agrees.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

int classify(int v)
{
    int r = 0;
    switch (v)
    {
    case 1:
        int shared = 10;
        r = shared;
        break;
    case 2:
        shared = 20;
        r = shared;
        break;
    default:
        r = 99;
        break;
    }
    return r;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = classify(2);
}
//CHECK: 20
