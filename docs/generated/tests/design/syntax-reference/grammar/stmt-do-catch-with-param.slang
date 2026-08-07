//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#statements
//META: doc_section_digest=f194f1b075af80ee23e030c52a1aedd7ee43bc2072f0ea3308cbffd439f98c56
//META: purpose=`DoCatchStmt` parses as `'do' Stmt 'catch' '(' Param ')' Stmt` with an explicit catch parameter.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the parameterized catch arm of DoCatchStmt: a throwing call
// inside `do` transfers control to `catch (e : PErr)`, whose handler
// value lands in the buffer. (Exception semantics are unreliable under
// INTERPRET, so this runs on the CPU compute path.)

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer

enum PErr
{
    Neg
}

func checked(x : int) throws PErr -> int
{
    if (x < 0)
    {
        throw PErr.Neg;
    }
    return x;
}

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    int r;
    do
    {
        r = try checked(-5);
    }
    catch (e : PErr)
    {
        r = 33;
    }
    // CHECK: 33
    outputBuffer[0] = r;
}
