// A non-extern `int f(int)` and an `extern float f(float)` are different overloads, not two
// declarations of one function: the preference for the definition over an `extern` declaration
// must not drop either of them. Each call must resolve to the overload that matches its argument,
// in both import orders. This file imports the non-extern overload first; see
// extern-overload-ext-first.slang for the swapped order.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -stage compute -entry computeMain

// No diagnostics are expected, and the check is exhaustive. Had the `extern float f(float)`
// candidate been dropped, `f(1.5f)` would have no applicable overload (`float` to `int` is not an
// implicit conversion); had `int f(int)` been dropped, `requireInteger(f(1))` would fail because
// `float` does not conform to `IInteger`. A call is ranked by conversion cost before the `extern`
// comparison is reached, so this file checks that both overloads stay callable; the bare-reference
// tests extern-overload-bare-*.slang drive the comparison itself.

import overloaddef;
import overloadext;

T requireInteger<T : IInteger>(T value)
{
    return value;
}

RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = f(1.5f);
    outputBuffer[1] = float(requireInteger(f(1)));
}
