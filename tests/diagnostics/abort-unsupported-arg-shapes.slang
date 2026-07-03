// Verify that non-scalar/ordinary-vector abort payload shapes are rejected with E55211.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv -capability abort -capability cooperative_vector -capability cooperative_matrix -capability subgroup_basic

Texture2D gTexture;
uniform uint* gPointer;

typealias SmallCoopMat =
    linalg::CoopMat<float, MemoryScope::Subgroup, 1, 1, linalg::CoopMatMatrixUse::MatrixA>;

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

[numthreads(1, 1, 1)]
void coopVectorCase(uint3 tid: SV_DispatchThreadID)
{
    CoopVec<float, 4> v = CoopVec<float, 4>(1.0, 2.0, 3.0, 4.0);
    abort("coopvec=%f", v);
}

[numthreads(1, 1, 1)]
void coopMatrixCase(uint3 tid: SV_DispatchThreadID)
{
    SmallCoopMat m = SmallCoopMat(1.0);
    abort("coopmat=%f", m);
}

//CHECK: E55211
//CHECK: E55211
//CHECK: E55211
//CHECK: E55211
//CHECK: E55211
//CHECK: E55211
