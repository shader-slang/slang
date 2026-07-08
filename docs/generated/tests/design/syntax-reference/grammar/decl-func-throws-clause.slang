//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:43:29Z
//META: source_commit=196bcf98f30aef1230ca33a5bada98c48c9cb7fb
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#function-style-declarations
//META: doc_section_digest=5e6e6830e19c77586abe7afcb70af45ad2c1dc56341e670eb4abae10eeb65319
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
