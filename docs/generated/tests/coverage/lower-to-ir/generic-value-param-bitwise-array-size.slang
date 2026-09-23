//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=Verifies `N ^ 1` and `~N` over a generic value parameter lower to constexpr bit-xor / bit-not IR ops and fold to 7 and 1 for N = 6.
//META: intent=characterization
//META: covers=source/slang/slang-lower-to-ir.cpp
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Pins the BitXor and BitNot arms of `visitBuiltinOperationIntVal`. An
// expression over a generic value parameter survives type checking as a
// `BuiltinOperationIntVal` rather than being folded, so lowering must emit the
// matching hoistable constexpr IR op keyed on the operator; the type checker
// then specializes it. Using each expression as an array bound is what forces
// it into an `IntVal` (type-level) position instead of an ordinary expression.
// The observations are the folded results for N = 6: `6 ^ 1` is 7 and
// `(~6) & 7` is 1, so the buffer holds 7 * 10 + 1.
//TEST:COMPARE_COMPUTE(filecheck-buffer=CPU):-cpu -output-using-type
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry computeMain -stage compute

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

struct XorBuf<let N : int>
{
    int data[N ^ 1];
    static int count() { return N ^ 1; }
}

struct NotBuf<let N : int>
{
    int data[(~N) & 7];
    static int count() { return (~N) & 7; }
}

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = XorBuf<6>.count() * 10 + NotBuf<6>.count();
}

//CPU: 71

// Each specialized bound folds to its own constant in the emitted HLSL.
//HLSL-DAG: return int(7);
//HLSL-DAG: return int(1);
