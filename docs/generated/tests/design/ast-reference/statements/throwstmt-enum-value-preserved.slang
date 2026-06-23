//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/generated/design/ast-reference/statements.md#throwstmt-and-catchstmt
//META: doc_section_digest=4beb3cdf4f4fc264b184a0269fd084212ac38cb61fe979ed8cba8421dba3f523
//META: purpose=A ThrowStmt's `expression` Expr is transferred to the matching CatchStmt's `errorVar` ParamDecl; the handler observes the originally thrown enum value distinctly across distinct enum members.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary: the documented CatchStmt holds `errorVar: ParamDecl*`
// so "the catch handler has a fully-typed local variable". We
// throw two distinct enum members from the same enum and observe
// that the caught value matches the thrown one. The interpreter
// does not faithfully model throw/catch, so we use CPU compute.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

enum Err
{
    Alpha = 11,
    Beta = 22,
}

int risky(int which) throws Err
{
    if (which == 0) { throw Err.Alpha; }
    if (which == 1) { throw Err.Beta; }
    return 0;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    int r0 = 0;
    do
    {
        r0 = try risky(0);
    }
    catch (e: Err)
    {
        r0 = int(e);
    }
    outputBuffer[0] = r0;

    int r1 = 0;
    do
    {
        r1 = try risky(1);
    }
    catch (e: Err)
    {
        r1 = int(e);
    }
    outputBuffer[1] = r1;
}
//CHECK: 11
//CHECK: 22
