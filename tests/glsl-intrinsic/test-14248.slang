//TEST:SIMPLE_EX(filecheck=CHECK_GLSL): -lang glsl tests/glsl-intrinsic/test-14248.slang -stage compute -entry computeMain -target glsl
//TEST:SIMPLE_EX(filecheck=CHECK_SPIR): -lang glsl tests/glsl-intrinsic/test-14248.slang -stage compute -entry computeMain -target spirv -emit-spirv-directly
//TEST:SIMPLE_EX(filecheck=CHECK_HLSL): -lang glsl tests/glsl-intrinsic/test-14248.slang -stage compute -entry computeMain -target hlsl -DTARGET_HLSL
//TEST:SIMPLE_EX(filecheck=CHECK_HLSL_FLOOR): -lang glsl tests/glsl-intrinsic/test-14248.slang -stage compute -entry computeMain -target hlsl -profile sm_5_0
//TEST:SIMPLE_EX(filecheck=CHECK_WGSL): -lang glsl tests/glsl-intrinsic/test-14248.slang -stage compute -entry computeMain -target wgsl
// Validate the emitted WGSL through Tint on CI (ignored where the downstream compiler is absent).
//TEST:SIMPLE_EX(filecheck=CHECK_WGSL_SPV): -lang glsl tests/glsl-intrinsic/test-14248.slang -stage compute -entry computeMain -target wgsl-spirv-asm
//TEST:SIMPLE_EX(filecheck=CHECK_METAL): -lang glsl tests/glsl-intrinsic/test-14248.slang -stage compute -entry computeMain -target metal
//TEST:SIMPLE_EX(filecheck=CHECK_CUDA): -lang glsl tests/glsl-intrinsic/test-14248.slang -stage compute -entry computeMain -target cuda
//TEST:SIMPLE_EX(filecheck=CHECK_CPP): -lang glsl tests/glsl-intrinsic/test-14248.slang -stage compute -entry computeMain -target cpp

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF): -llvm -compute -entry computeMain -source-language glsl -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-cpu -compute -shaderobj -entry computeMain -source-language glsl -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -compute -entry computeMain -source-language glsl -output-using-type -emit-spirv-directly
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-wgpu -compute -shaderobj -entry computeMain -source-language glsl -output-using-type

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
buffer B
{
    int result;
} outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // CHECK_GLSL: inverse(
    // CHECK_SPIR: MatrixInverse{{ }}
    // E41012 is the implicit profile-upgrade diagnostic; the widened requires
    // must not raise any newly admitted target's capability floor.
    // CHECK_HLSL-NOT: E41012
    // The count is 4 = the 2x2/3x3/4x4 cases plus the TARGET_HLSL-only 1x1
    // case below; the fallback lowers each inverse() to a native determinant.
    // CHECK_HLSL-COUNT-4: determinant(
    // CHECK_WGSL-NOT: E41012
    // CHECK_WGSL: determinant(
    // CHECK_METAL-NOT: E41012
    // CHECK_METAL: determinant(
    // CHECK_CUDA-NOT: E41012
    // CHECK_CUDA: void computeMain(
    // CHECK_CPP-NOT: E41012
    // CHECK_CPP: void _computeMain(
    // The sm_5_0 floor leg pins the deliberate GLSL_400 convention: older
    // profiles are implicitly upgraded (E41012), matching the sibling
    // intrinsics gated the same way.
    // CHECK_HLSL_FLOOR: E41012
    // CHECK_WGSL_SPV: computeMain
    // BUF: 1

    // The fallback must reproduce exact results for matrices whose inverse
    // is representable in floating point, so every check below is ==.
    bool ok = true;

    // 2x2: inverse of diag(2, 4) is diag(0.5, 0.25).
    const mat2 m2 = mat2(2.0, 0.0, 0.0, 4.0);
    const mat2 i2 = inverse(m2);
    ok = ok
        && i2[0][0] == 0.5
        && i2[0][1] == 0.0
        && i2[1][0] == 0.0
        && i2[1][1] == 0.25;

    // 3x3: unit triangular; its inverse is integral.
    const mat3 m3 = mat3(1.0, 0.0, 0.0, 2.0, 1.0, 0.0, 0.0, 4.0, 1.0);
    const mat3 i3 = inverse(m3);
    ok = ok
        && i3[0][0] == 1.0 && i3[1][0] == -2.0 && i3[2][0] == 8.0
        && i3[0][1] == 0.0 && i3[1][1] == 1.0 && i3[2][1] == -4.0
        && i3[0][2] == 0.0 && i3[1][2] == 0.0 && i3[2][2] == 1.0;

    // 4x4: unit triangular; its inverse is integral.
    const mat4 m4 =
        mat4(1.0, 0.0, 0.0, 0.0, 2.0, 1.0, 0.0, 0.0, 0.0, 4.0, 1.0, 0.0, 0.0, 0.0, 8.0, 1.0);
    const mat4 i4 = inverse(m4);
    ok = ok
        && i4[0][0] == 1.0 && i4[1][0] == -2.0 && i4[2][0] == 8.0 && i4[3][0] == -64.0
        && i4[0][1] == 0.0 && i4[1][1] == 1.0 && i4[2][1] == -4.0 && i4[3][1] == 32.0
        && i4[0][2] == 0.0 && i4[1][2] == 0.0 && i4[2][2] == 1.0 && i4[3][2] == -8.0
        && i4[0][3] == 0.0 && i4[1][3] == 0.0 && i4[2][3] == 0.0 && i4[3][3] == 1.0;

#if defined(TARGET_HLSL)
    // The E39999 codegen gate rejects 1x1 matrices on every target except
    // HLSL, so only that compile leg runs this case.
    const matrix<float, 1, 1> m1 = matrix<float, 1, 1>(2.0);
    const matrix<float, 1, 1> i1 = inverse(m1);
    ok = ok && i1[0][0] == 0.5;
#endif

    outputBuffer.result = int(ok);
}
