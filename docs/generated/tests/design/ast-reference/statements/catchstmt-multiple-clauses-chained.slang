//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/statements.md#throwstmt-and-catchstmt
//META: doc_section_digest=0c9bef4794f61ac3fead35d16b0cd0603f6881658db01263a9dfdf11e63c7f46
//META: purpose=A `do` body may be followed by several `catch` clauses; the clauses chain so that each later handler protects the one before it, and an error raised in the body is taken by the first matching handler.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies two `catch` clauses after one `do` body are both accepted
// and that the typed handler nearest the body wins. The protected body
// always throws; the first handler writes 7 and the trailing catch-all
// writes 8. The CHECK asserts 7, so the chain did not route the error
// past the matching handler.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

enum ErrA { Bad }

int risky(int n) throws ErrA
{
    if (n < 0) { throw ErrA.Bad; }
    return n;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    int r = 0;
    do
    {
        r = try risky(-1);
    }
    catch (e: ErrA)
    {
        r = 7;
    }
    catch
    {
        r = 8;
    }
    outputBuffer[0] = r;
}
//CHECK: 7
