//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#statement-keywords
//META: doc_section_digest=1fb3516e49ddbb5d4259751275e6b594d9d69aa0b1266c092e549ec5de2b449a
//META: purpose=The statement-level handler is `do { ... } catch ( ... ) { ... }` and its error parameter is optional, so a bare `catch` with no parameter list catches every error type.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies `catch` pairs with `do` (not with `try`) at statement level
// and that the parameter list may be omitted entirely. `run` wraps a
// throwing call in `do { ... } catch { ... }` with no error binding.
// Slot 0 takes the non-throwing path and must hold the returned 5;
// slot 1 takes the throwing path and must hold 99, which is reachable
// only if the parameterless handler still caught the thrown value.
// Exception semantics are observed on the CPU target because the
// bytecode interpreter does not model them faithfully.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

enum MyError
{
    Fail
};

int mayFail(int n) throws MyError
{
    if (n == 1)
        throw MyError.Fail;
    return 5;
}

int run(int n)
{
    do
    {
        return try mayFail(n);
    }
    catch
    {
        return 99;
    }
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(int3 tid: SV_DispatchThreadID)
{
    outputBuffer[0] = run(0);
    outputBuffer[1] = run(1);
}

// CHECK: {{^}}5
// CHECK-NEXT: {{^}}99
