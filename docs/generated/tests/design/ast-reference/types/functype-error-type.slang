//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ast-reference/types.md#functype
//META: doc_section_digest=7e903f116cccaa2d6cdd7b7b3629e179d6417c0f583ea8c8d54a18facbc96121
//META: purpose=A FuncType carries an optional error type for a throwing callable; the value on the success path flows through when no error is thrown.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the FuncType claim that it carries an "optional error
// type" for errorable callables. A `throws MyError` function that
// returns normally threads its result to the caller through `try`;
// the success value is written to an output buffer. CHECK asserts
// that value. (INTERPRET misevaluates catch, so this uses
// COMPARE_COMPUTE per _common.md.)

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outBuf
RWStructuredBuffer<int> outBuf;

enum MyError { Fail = 0x10 }

int mightFail(int x) throws MyError
{
    if (x < 0) throw MyError.Fail;
    return x + 1;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    int r = 0;
    do { r = try mightFail(41); }
    catch (e: MyError) { r = -1; }
    // CHECK: 42
    outBuf[0] = r;
}
