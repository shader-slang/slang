//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#function-style-declarations
//META: doc_section_digest=a505032fe1ced813d290018b087d7ba67b9a672a9b25244d2cf6a5fa887fe795
//META: purpose=`FuncDecl` accepts a `('throws' Type)?` clause between the parameter list and the `->` result type.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the `throws` clause position in a `func` header:
// `func halveIt(x : int) throws HalfErr -> int` (named to avoid the
// builtin `half` type, which would shadow the call). The success path is
// exercised through `try` inside a `do ... catch`, so the observed 42
// proves both the header parsed and the callable is usable.
// (Exception semantics are unreliable under INTERPRET, so this runs
// on the CPU compute path.)

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer

enum HalfErr
{
    Odd
}

func halveIt(x : int) throws HalfErr -> int
{
    if ((x & 1) != 0)
    {
        throw HalfErr.Odd;
    }
    return x / 2;
}

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    int r;
    do
    {
        r = try halveIt(84);
    }
    catch (e : HalfErr)
    {
        r = -1;
    }
    // CHECK: 42
    outputBuffer[0] = r;
}
