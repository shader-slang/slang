//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry hlslMain -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv -entry spirvMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=FORWARD,non-exhaustive):-target spirv -entry forwardMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=BACKWARD,non-exhaustive):-target spirv -entry backwardMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=FORWARD_UNMATCHED,non-exhaustive):-target hlsl -entry forwardUnmatchedMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=BACKWARD_UNMATCHED,non-exhaustive):-target hlsl -entry backwardUnmatchedMain -stage compute
//TEST:SIMPLE(filecheck=SKIPPED):-target hlsl -entry backwardSkippedMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=COMPUTED,non-exhaustive):-target hlsl -entry backwardComputedMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=OWNER,non-exhaustive):-target hlsl -entry backwardOwnerMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=COMPUTED_UNMATCHED,non-exhaustive):-target hlsl -entry backwardComputedUnmatchedMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=OWNER_UNMATCHED,non-exhaustive):-target hlsl -entry backwardOwnerUnmatchedMain -stage compute
// non-exhaustive: integer conversions warn before specialization and errors include notes.

enum Mode
{
    [require(hlsl)] hlslMode = 0,
    [require(spirv)] spirvMode = 1,
}

struct Holder<let mode : Mode>
{
    int payload;
}

[Differentiable]
float square<let tag : int>(float x)
{
    // The differentiated callable retains this source requirement without a numerical derivative.
    const Mode mode = Mode(tag);
    return x * x;
}

float readForward<let tag : int>()
{
    return fwd_diff(square<tag>)(diffPair(2.0, 1.0)).d;
}

float readBackward<let tag : int>()
{
    var x = diffPair(2.0, 0.0);
    bwd_diff(square<tag>)(x, 1.0);
    return x.d;
}

[Differentiable]
float squareSkipped<let tag : int>(float x)
{
    if (tag < 0)
    {
        const Mode mode = Mode(tag);
    }
    return x * x;
}

[Differentiable]
float squareComputed<let tag : int>(float x)
{
    const Mode mode = Mode(tag + 1);
    return x * x;
}

[Differentiable]
float squareOwner<let tag : int>(float x)
{
    Holder<Mode(tag + 1)> value;
    return x * x;
}

float readBackwardComputed<let tag : int>()
{
    var x = diffPair(2.0, 0.0);
    bwd_diff(squareComputed<tag>)(x, 1.0);
    return x.d;
}

float readBackwardOwner<let tag : int>()
{
    var x = diffPair(2.0, 0.0);
    bwd_diff(squareOwner<tag>)(x, 1.0);
    return x.d;
}

RWStructuredBuffer<float> outputBuffer;

// HLSL: void hlslMain
[numthreads(1, 1, 1)]
void hlslMain()
{
    outputBuffer[0] = readForward<0>();
    outputBuffer[1] = readBackward<0>();
    outputBuffer[2] = readBackwardComputed<-1>();
    outputBuffer[3] = readBackwardOwner<-1>();
}

// SPIRV: OpEntryPoint GLCompute
[numthreads(1, 1, 1)]
void spirvMain()
{
    outputBuffer[0] = readForward<1>();
    outputBuffer[1] = readBackward<1>();
    outputBuffer[2] = readBackwardComputed<0>();
    outputBuffer[3] = readBackwardOwner<0>();
}

[numthreads(1, 1, 1)]
void forwardMain()
//FORWARD: enum conversion selects an unavailable case
{
    outputBuffer[0] = readForward<0>();
}

[numthreads(1, 1, 1)]
void backwardMain()
//BACKWARD: enum conversion selects an unavailable case
{
    outputBuffer[0] = readBackward<0>();
}

[numthreads(1, 1, 1)]
void forwardUnmatchedMain()
//FORWARD_UNMATCHED: enum conversion has no matching case after specialization
{
    outputBuffer[0] = readForward<42>();
}

[numthreads(1, 1, 1)]
void backwardUnmatchedMain()
//BACKWARD_UNMATCHED: enum conversion has no matching case after specialization
{
    outputBuffer[0] = readBackward<42>();
}

// SKIPPED: void backwardSkippedMain
[numthreads(1, 1, 1)]
void backwardSkippedMain()
{
    // The cast is unreachable after specialization; differentiation must not hoist its check.
    var x = diffPair(2.0, 0.0);
    bwd_diff(squareSkipped<42>)(x, 1.0);
    outputBuffer[0] = x.d;
}

[numthreads(1, 1, 1)]
void backwardComputedMain()
//COMPUTED: enum conversion selects an unavailable case
{
    outputBuffer[0] = readBackwardComputed<0>();
}

[numthreads(1, 1, 1)]
void backwardOwnerMain()
//OWNER: enum conversion selects an unavailable case
{
    outputBuffer[0] = readBackwardOwner<0>();
}

[numthreads(1, 1, 1)]
void backwardComputedUnmatchedMain()
//COMPUTED_UNMATCHED: enum conversion has no matching case after specialization
{
    outputBuffer[0] = readBackwardComputed<41>();
}

[numthreads(1, 1, 1)]
void backwardOwnerUnmatchedMain()
//OWNER_UNMATCHED: enum conversion has no matching case after specialization
{
    outputBuffer[0] = readBackwardOwner<41>();
}
