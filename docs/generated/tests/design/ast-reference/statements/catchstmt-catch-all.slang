//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/generated/design/ast-reference/statements.md#throwstmt-and-catchstmt
//META: doc_section_digest=4beb3cdf4f4fc264b184a0269fd084212ac38cb61fe979ed8cba8421dba3f523
//META: purpose=A CatchStmt with a null `errorVar` is a catch-all that does not bind the error value; it catches errors of any type from the protected body.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says "A null errorVar denotes a catch-all that does not
// bind the error value". The surface form is a `catch { ... }` with
// no parameter. We observe by throwing an error and catching it
// without binding the value: the handler must still run.

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
