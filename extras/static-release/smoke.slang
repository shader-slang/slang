// Trivial shader used to smoke-test a statically linked slangc.
//
// It exists to give the SPIR-V backend something to chew on at -O3, which is
// the path that reaches into the embedded glslang/SPIRV-Tools wrapper. At -O0
// Slang emits SPIR-V natively and would never touch it, so a shader this small
// is still enough to tell a working SLANG_EMBED_SLANG_GLSLANG build from a
// broken one.

RWStructuredBuffer<float> result;

[shader("compute")]
[numthreads(4, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    result[tid.x] = float(tid.x) * 2.0f;
}
