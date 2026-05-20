//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/llm-generated/ast-reference/statements.md#throwstmt-and-catchstmt
//META: doc_section_digest=4beb3cdf4f4fc264b184a0269fd084212ac38cb61fe979ed8cba8421dba3f523
//META: purpose=Nested CatchStmt blocks resolve to the innermost matching handler; a throw inside an inner try is caught by the inner handler and the outer handler does not fire.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary: nested try/catch shape. The doc says a CatchStmt
// "holds both the protected body (tryBody) and the handler
// (handleBody)". Two nested CatchStmt forms must resolve a thrown
// error against the innermost protected body that surrounds the
// throw. We tag the inner handler with 11 and the outer with 22:
// only the inner tag is observed.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

enum Err { Fail }

int risky() throws Err
{
    throw Err.Fail;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    int r = 0;
    do
    {
        do
        {
            r = try risky();
        }
        catch (inner: Err)
        {
            r = 11;
        }
    }
    catch (outer: Err)
    {
        r = 22;
    }
    outputBuffer[0] = r;
}
//CHECK: 11
