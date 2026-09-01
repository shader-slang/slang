//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/statements.md#throwstmt-and-catchstmt
//META: doc_section_digest=0c9bef4794f61ac3fead35d16b0cd0603f6881658db01263a9dfdf11e63c7f46
//META: purpose=There is no `try` statement: a `try` written at statement position is parsed as an expression statement, so `try f();` is a call whose result is discarded while the error still propagates to the caller's handler.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies `try f();` standing alone is an ordinary expression
// statement rather than a statement form of its own: the wrapper
// discards the call's result but keeps propagating the error. The
// wrapper is called twice, once on a succeeding path and once on a
// throwing path; the CHECKs assert the wrapper's own 5 for the first
// and the caller's handler value -1 for the second.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

enum ErrA { Bad }

int risky(int n) throws ErrA
{
    if (n < 0) { throw ErrA.Bad; }
    return n;
}

int wrapper(int n) throws ErrA
{
    try risky(n);
    return 5;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    int good = 0;
    do { good = try wrapper(1); } catch { good = -1; }
    outputBuffer[0] = good;

    int bad = 0;
    do { bad = try wrapper(-1); } catch { bad = -1; }
    outputBuffer[1] = bad;
}
//CHECK: 5
//CHECK: -1
