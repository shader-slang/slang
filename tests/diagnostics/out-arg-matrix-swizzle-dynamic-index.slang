//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -entry computeMain -stage compute

// Dynamic subscripts of matrix/vector swizzle l-values are not yet assignable.
// Diagnose them explicitly instead of falling through to the out-argument
// write-back path and ICEing during IR lowering.

void writeOut(out float x, float value)
{
    x = value;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    float4x4 m = float4x4(0);
    writeOut(m._41_32[tid.x], 7.0);
//CHECK:                  ^ expected a compile-time constant
}
