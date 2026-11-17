//TEST:SIMPLE_EX(filecheck=CHECK_GLSL): -lang glsl tests/glsl-intrinsic/test-14248.slang -stage compute -entry computeMain -target glsl
//TEST:SIMPLE_EX(filecheck=CHECK_SPIR): -lang glsl tests/glsl-intrinsic/test-14248.slang -stage compute -entry computeMain -target spirv -emit-spirv-directly
//TEST:SIMPLE_EX(filecheck=CHECK_HLSL): -lang glsl tests/glsl-intrinsic/test-14248.slang -stage compute -entry computeMain -target hlsl
//TEST:SIMPLE_EX: -lang glsl tests/glsl-intrinsic/test-14248.slang -stage compute -entry computeMain -target wgsl -o test-14248.wgsl
//TEST:SIMPLE_EX: -lang glsl tests/glsl-intrinsic/test-14248.slang -stage compute -entry computeMain -target metal -o test-14248.metal
//TEST:SIMPLE_EX: -lang glsl tests/glsl-intrinsic/test-14248.slang -stage compute -entry computeMain -target cuda -o test-14248.cu
//TEST:SIMPLE_EX: -lang glsl tests/glsl-intrinsic/test-14248.slang -stage compute -entry computeMain -target cpp -o test-14248.cpp

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF): -llvm -compute -entry computeMain -source-language glsl -output-using-type

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
    // CHECK_HLSL: void computeMain(
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

    // 3x3: unit upper triangular; its inverse is integral.
    const mat3 m3 = mat3(1.0, 0.0, 0.0, 2.0, 1.0, 0.0, 0.0, 4.0, 1.0);
    const mat3 i3 = inverse(m3);
    ok = ok
        && i3[0][0] == 1.0 && i3[1][0] == -2.0 && i3[2][0] == 8.0
        && i3[0][1] == 0.0 && i3[1][1] == 1.0 && i3[2][1] == -4.0
        && i3[0][2] == 0.0 && i3[1][2] == 0.0 && i3[2][2] == 1.0;

    // 4x4: unit upper triangular; its inverse is integral.
    const mat4 m4 =
        mat4(1.0, 0.0, 0.0, 0.0, 2.0, 1.0, 0.0, 0.0, 0.0, 4.0, 1.0, 0.0, 0.0, 0.0, 8.0, 1.0);
    const mat4 i4 = inverse(m4);
    ok = ok
        && i4[0][0] == 1.0 && i4[1][0] == -2.0 && i4[2][0] == 8.0 && i4[3][0] == -64.0
        && i4[0][1] == 0.0 && i4[1][1] == 1.0 && i4[2][1] == -4.0 && i4[3][1] == 32.0
        && i4[0][2] == 0.0 && i4[1][2] == 0.0 && i4[2][2] == 1.0 && i4[3][2] == -8.0
        && i4[0][3] == 0.0 && i4[1][3] == 0.0 && i4[2][3] == 0.0 && i4[3][3] == 1.0;

    outputBuffer.result = int(ok);
}
