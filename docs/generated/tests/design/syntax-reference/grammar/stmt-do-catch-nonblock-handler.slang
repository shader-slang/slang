//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#statements
//META: doc_section_digest=11f48bb43fb7900ef7dd3c2f4785da1cab7aaae9b90d2da44ecf5e8cf8e950b1
//META: purpose=`DoCatchStmt`'s handler is a general `Stmt`, not only a `Block`; a single expression statement is accepted.
//META: intent=boundary
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the catch handler position takes any Stmt: here a bare
// expression statement (`r = 55;`) rather than a braced block. The
// handler assignment is observed in the buffer. (Exception semantics
// are unreliable under INTERPRET, so this runs on the CPU compute
// path.)

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
    int r = 0;
    do
    {
        r = try checked(-1);
    }
    catch (e : PErr)
        r = 55;
    // CHECK: 55
    outputBuffer[0] = r;
}
