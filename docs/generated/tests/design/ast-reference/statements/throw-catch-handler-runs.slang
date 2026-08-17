//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/statements.md#throwstmt-and-catchstmt
//META: doc_section_digest=0c9bef4794f61ac3fead35d16b0cd0603f6881658db01263a9dfdf11e63c7f46
//META: purpose=A ThrowStmt in an errorable function transfers control to the caller's matching CatchStmt; the catch handler runs and the protected body's remaining statements are skipped.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies `throw` in an errorable function transfers control to the
// caller's handler, while the non-throwing path returns normally. The
// CHECKs assert 105 for the successful call and -1 for the failing one,
// which separates the protected body from the handler.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

enum MyError { Fail }

int risky(int n) throws MyError
{
    if (n < 0) { throw MyError.Fail; }
    return n + 100;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    int r1 = 0;
    do
    {
        r1 = try risky(5);
    }
    catch (e: MyError)
    {
        r1 = -1;
    }
    outputBuffer[0] = r1;

    int r2 = 0;
    do
    {
        r2 = try risky(-1);
    }
    catch (e: MyError)
    {
        r2 = -1;
    }
    outputBuffer[1] = r2;
}
//CHECK: 105
//CHECK: -1
