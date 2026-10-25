// The zero-argument abort() (HLSL SM 4.0) exists only on HLSL and C++. On the other targets it used
// to be emitted as an undefined `abort()` call (and failed to parse as a SPIR-V intrinsic); it is
// now a capability diagnostic. The variadic abort(format, ...) is the Vulkan form.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target wgsl -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target metal -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target glsl -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv -entry computeMain -stage compute

RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    if (tid.x > 100)
    {
        abort();
    }
    outputBuffer[0] = 1;
}

//CHECK: E36107
