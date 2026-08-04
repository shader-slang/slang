//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/generated/design/ast-reference/statements.md#throwstmt-and-catchstmt
//META: doc_section_digest=4beb3cdf4f4fc264b184a0269fd084212ac38cb61fe979ed8cba8421dba3f523
//META: purpose=A ThrowStmt in an errorable function transfers control to the caller's matching CatchStmt; the catch handler runs and the protected body's remaining statements are skipped.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says ThrowStmt and CatchStmt are "the two halves of Slang's
// errorable-function model" and a CatchStmt "holds both the protected
// body (tryBody) and the handler (handleBody)". We observe both
// halves: a normal call returns 105 (no throw), and a failing call
// triggers the handler that writes -1. The interpreter does not
// faithfully model throw/catch, so we use CPU compute as the runtime.

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
