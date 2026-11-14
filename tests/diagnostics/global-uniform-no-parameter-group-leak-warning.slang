// Regression test for https://github.com/shader-slang/slang/issues/11825
//
// Global-scope shader parameters are gathered by the compiler into a synthesized
// `ConstantBuffer<GlobalParams>` (via `collectGlobalUniformParameters`). This is a
// distinct synthesis site from the entry-point parameter collection exercised by
// `entry-point-uniform-no-parameter-group-leak-warning.slang`. When a resource then
// "leaks" out of that constant buffer during type legalization we must NOT warn with
// E31106/E31107: the user wrote a flat set of globals, not a parameter group, so
// there is nothing for them to restructure and the warning (which also came with no
// source location) is just noise.
//
// The test is exhaustive (no `non-exhaustive`): if any diagnostic is produced it must
// be annotated below, so the absence of annotations asserts that compiling this entry
// point is completely silent.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -entry mainGlobal
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -entry mainGlobal

Texture2D<float4> gTexture;
RWTexture2D<float4> gRwTexture;
uniform int2 gDim;

[shader("compute")]
[numthreads(1, 1, 1)]
void mainGlobal(uint3 tid : SV_DispatchThreadID)
{
    if (all(tid.xy < gDim))
        gRwTexture[tid.xy] = gTexture[tid.xy];
}
