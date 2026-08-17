// Regression test for https://github.com/shader-slang/slang/issues/11825
//
// Entry-point `uniform`/resource parameters are gathered by the compiler into a
// synthesized `ConstantBuffer<EntryPointParams>`. When a resource then "leaks"
// out of that constant buffer during type legalization we must NOT warn with
// E31106/E31107: the user wrote a flat parameter list, not a parameter group,
// so there is nothing for them to restructure and the warning (which also came
// with no source location) is just noise.
//
// The test is exhaustive (no `non-exhaustive`): if any diagnostic is produced it
// must be annotated below, so the absence of annotations asserts that compiling
// this entry point is completely silent.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -entry testMain
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -entry testMain

[shader("compute")]
[numthreads(1, 1, 1)]
void testMain(
    uint3 tid : SV_DispatchThreadID,
    Texture2D<float4> texture,
    RWTexture2D<float4> rw_texture,
    uniform int2 dim)
{
    if (all(tid.xy < dim))
        rw_texture[tid.xy] = texture[tid.xy];
}
