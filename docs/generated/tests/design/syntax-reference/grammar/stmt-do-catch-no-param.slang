//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:43:29Z
//META: source_commit=196bcf98f30aef1230ca33a5bada98c48c9cb7fb
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#statements
//META: doc_section_digest=c4108d6661545bac260ff71845e7ba2fe35ca7f7587b33e25c895a81925cb89d
//META: purpose=`DoCatchStmt` accepts an omitted catch parameter: `'do' Stmt 'catch' Stmt` with no `'(' Param ')'`.
//META: intent=boundary
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the optional-parameter side of the DoCatchStmt production:
// a bare `catch { ... }` (no parenthesized parameter) still parses and
// handles the error raised inside the `do` body. (Exception semantics
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
    int r;
    do
    {
        r = try checked(-5);
    }
    catch
    {
        r = 44;
    }
    // CHECK: 44
    outputBuffer[0] = r;
}
