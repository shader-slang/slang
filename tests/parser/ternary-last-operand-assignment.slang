//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv

// The last operand of `?:` is parsed at the `?:` precedence level, so this is
// `(c ? 0.0 : b) /= 2.0` and fails because a `?:` is not an l-value. This test records the current
// behavior pending a maintainer decision on the last operand (shader-slang/slang#13870); it does not
// state the intended grammar. If Slang adopts the C++/GLSL rule, this line parses as
// `c ? 0.0 : (b /= 2.0)` and the test should be inverted.

RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    float b = outputBuffer[0];
    bool c = b > 1.0;
    c ? 0.0 : b /= 2.0;
/*CHECK:
      ^ argument must be l-value
      ^ argument passed to parameter '0' must be l-value.
*/
    outputBuffer[1] = b;
}
