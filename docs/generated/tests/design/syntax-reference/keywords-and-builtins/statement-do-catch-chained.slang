//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#statement-keywords
//META: doc_section_digest=58adeae3c01b07a208e3d126aee114b032a435add75925b7e18289c39283719b
//META: purpose=A `do` statement may be followed by a chain of `catch` clauses; the parser keeps consuming further `catch` clauses and nests each one inside the next, so every error type in the chain reaches its own handler.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the multi-`catch` chain form. `run` guards two calls that
// throw two different enum error types and supplies one typed handler
// per type. Slot 0 exercises the success path, slot 1 forces the first
// error type, slot 2 forces the second. CHECK asserts 3, 11, 22 in
// order: each distinct value can only appear if the chain routed the
// thrown value to the handler whose parameter type matches, which
// requires the second `catch` to have been consumed as part of the
// same statement rather than treated as a stray token.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

enum ErrA
{
    Bad
};

enum ErrB
{
    Worse
};

int failsWithA(int n) throws ErrA
{
    if (n == 1)
        throw ErrA.Bad;
    return 1;
}

int failsWithB(int n) throws ErrB
{
    if (n == 2)
        throw ErrB.Worse;
    return 2;
}

int run(int n)
{
    do
    {
        return try failsWithA(n) + try failsWithB(n);
    }
    catch (e: ErrA)
    {
        return 11;
    }
    catch (e: ErrB)
    {
        return 22;
    }
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(int3 tid: SV_DispatchThreadID)
{
    outputBuffer[0] = run(0);
    outputBuffer[1] = run(1);
    outputBuffer[2] = run(2);
}

// CHECK: {{^}}3
// CHECK-NEXT: {{^}}11
// CHECK-NEXT: {{^}}22
