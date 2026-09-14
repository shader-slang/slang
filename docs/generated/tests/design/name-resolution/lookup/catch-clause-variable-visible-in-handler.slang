//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/name-resolution/lookup.md#block-local-shadowing
//META: doc_section_digest=fff04d2b441a5ad2e4ef387152ddc76216942a0668864eb5cdfb1c9b98e7e532
//META: purpose=A catch clause's error variable is cleared of the block-local hidden flag before its handler is checked, so the handler body can refer to it.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the third writer of the block-local hidden flag. A local
// declared in a block is hidden until the checker walks past its own
// declaration statement, but a catch clause's error variable has no
// declaration statement inside the handler block, so it is cleared
// when the clause itself is checked. The handler here reads `e` on the
// first statement of its body, which would be an undefined identifier
// if the flag were still set. Exception semantics are not modelled by
// the interpreter, so the observation is a CPU compute run: CHECK
// asserts 43, the thrown 42 plus one, proving the handler read the
// caught value rather than falling through to the untouched initial 0.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

int mayThrow(int x) throws int
{
    if (x > 0)
        throw 42;
    return 7;
}

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    int r = 0;
    do
    {
        r = try mayThrow(1);
    }
    catch (int e)
    {
        r = e + 1;
    }
    outputBuffer[0] = r;
    // CHECK: 43
}
