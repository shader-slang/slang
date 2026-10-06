// `readonly` on a parameter without `__ref` stays the GLSL memory qualifier, so on a
// non-image parameter it is still rejected, exactly as before `readonly __ref` existed.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -entry computeMain -stage compute

uint takesValue(readonly uint v)
//CHECK:                      ^ E31206
//CHECK:                      ^ modifier readonly is not allowed on a non image type parameter.
{
    return v;
}

RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = takesValue(outputBuffer[0]);
}
