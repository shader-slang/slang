// Verify that non-scalar/vector abort payload shapes are rejected with E55211.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv -capability abort -skip-spirv-validation

Texture2D gTexture;
uniform uint* gPointer;

[numthreads(1, 1, 1)]
void matrixCase(uint3 tid: SV_DispatchThreadID)
{
    float2x2 m = float2x2(1.0, 2.0, 3.0, 4.0);
    abort("m=%f", m);
}

[numthreads(1, 1, 1)]
void arrayCase(uint3 tid: SV_DispatchThreadID)
{
    uint values[4] = { tid.x, tid.y, tid.z, 0 };
    abort("values=%u", values);
}

[numthreads(1, 1, 1)]
void resourceCase(uint3 tid: SV_DispatchThreadID)
{
    abort("tex=%u", gTexture);
}

[numthreads(1, 1, 1)]
void pointerCase(uint3 tid: SV_DispatchThreadID)
{
    abort("ptr=%u", gPointer);
}

//CHECK: E55211
//CHECK: E55211
//CHECK: E55211
//CHECK: E55211
