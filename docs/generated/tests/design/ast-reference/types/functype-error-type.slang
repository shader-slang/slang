//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/types.md#functype
//META: doc_section_digest=5e92206b0f0b0b7cbc40b1055a81166a8628747c755d5cf61ab0cfb9b61096cf
//META: purpose=A FuncType's operand list ends with an error type; a callable declared `throws E` threads its ordinary result to the caller when no error is raised.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the error slot of a callable type: a `throws MyError`
// function has a distinct error type but still carries a normal result.
// The success path is taken and its value written to a buffer; CHECK
// asserts 42, so a callable whose result had been displaced by the
// error type would fail. COMPARE_COMPUTE is used because slangi
// misevaluates catch.

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
