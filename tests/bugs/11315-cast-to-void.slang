// Regression test for:
// https://github.com/shader-slang/slang/issues/11315
//
// `(void)expr;` lowers to a `kIROp_CastToVoid` IR instruction that no
// emit backend handles, so every target previously aborted with an
// internal "unimplemented" error. The fix drops the no-op cast in a
// pre-emit pass; this test pins the bug across SPIR-V and one text
// emit target so it cannot silently regress.

//TEST:SIMPLE(filecheck=CHECK_SPIRV):-target spirv-asm -entry main -stage compute -emit-spirv-directly
//TEST:SIMPLE(filecheck=CHECK_HLSL):-target hlsl -entry main -stage compute

// CHECK_SPIRV: OpEntryPoint
// CHECK_HLSL: void main

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    int x = 5;
    (void)x;
}
