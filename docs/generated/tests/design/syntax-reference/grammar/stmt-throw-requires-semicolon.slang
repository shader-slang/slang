//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#statements
//META: doc_section_digest=f194f1b075af80ee23e030c52a1aedd7ee43bc2072f0ea3308cbffd439f98c56
//META: purpose=`ThrowStmt` parses as `'throw' Expr ';'`; the terminating semicolon is mandatory.
//META: intent=boundary
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that ThrowStmt is terminated by a mandatory `;`, like the other
// simple statements: `ParseThrowStatement` ends with
// `ReadToken(TokenType::Semicolon)`, so omitting it is a parse error rather
// than an empty statement. The raised error is observed via the catch handler
// value. (Exception semantics are unreliable under INTERPRET, so this runs on
// the CPU compute path.)

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer

enum TErr
{
    Boom
}

func trip(x : int) throws TErr -> int
{
    if (x > 0)
    {
        throw TErr.Boom;
    }
    return 0;
}

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    int r;
    do
    {
        r = try trip(1);
    }
    catch (e : TErr)
    {
        r = 66;
    }
    // CHECK: 66
    outputBuffer[0] = r;
}
