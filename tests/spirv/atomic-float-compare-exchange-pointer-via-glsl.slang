// GLSL has no pointer bitcast, so a floating-point atomic compare-exchange through a physical
// storage buffer pointer reports E55204 there. That includes SPIR-V emitted through GLSL, where the
// code that is emitted is GLSL (see atomic-float-compare-exchange-pointer.slang for direct SPIR-V).

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK, non-exhaustive): -target spirv -entry computeMain -stage compute -emit-spirv-via-glsl
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK, non-exhaustive): -target glsl -entry computeMain -stage compute

uniform Atomic<float>* floats;
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = floats[0].compareExchange(0.0f, 1.0f);
//CHECK:                                       ^ floating point atomic compare-exchange outside a RWStructuredBuffer
}
