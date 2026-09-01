//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/statements.md#throwstmt-and-catchstmt
//META: doc_section_digest=0c9bef4794f61ac3fead35d16b0cd0603f6881658db01263a9dfdf11e63c7f46
//META: purpose=A CatchStmt with a null `errorVar` is a catch-all that does not bind the error value; it catches errors of any type from the protected body.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies a handler written without an error parameter catches the
// error without binding its value. The protected body throws, so the
// only way the buffer can hold the handler's 42 is if the unbound
// handler ran.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

enum MyError { Fail }

int risky() throws MyError
{
    throw MyError.Fail;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    int r = 0;
    do
    {
        r = try risky();
    }
    catch
    {
        r = 42;
    }
    outputBuffer[0] = r;
}
//CHECK: 42
