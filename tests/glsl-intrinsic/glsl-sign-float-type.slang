// Regression test: GLSL sign(genType) must return genType (float / vecN / double),
// not int. The old behaviour used the HLSL sign() overloads which return int, so
//
//   sign(x) / 2        (with x = -3.0)
//
// performed integer division and produced 0 instead of the correct -0.5.
//
// GLSL spec (§8.3): sign(genType x) → genType, sign(genIType x) → genIType.
// In SPIR-V this means FSign (not SSign) with a float result type, with no
// surrounding ConvertFToS / ConvertSToF round-trip.

// --- Codegen checks (no GPU required) ---

//TEST:SIMPLE_EX(filecheck=SPIR): -lang glsl tests/glsl-intrinsic/glsl-sign-float-type.slang -stage compute -entry computeMain -target spirv -emit-spirv-directly
//TEST:SIMPLE_EX(filecheck=SPIR_NO_INT_CAST): -lang glsl tests/glsl-intrinsic/glsl-sign-float-type.slang -stage compute -entry computeMain -target spirv -emit-spirv-directly
//TEST:SIMPLE_EX(filecheck=GLSL): -lang glsl tests/glsl-intrinsic/glsl-sign-float-type.slang -stage compute -entry computeMain -target glsl
//TEST:SIMPLE_EX(filecheck=WGSL): -lang glsl tests/glsl-intrinsic/glsl-sign-float-type.slang -stage compute -entry computeMain -target wgsl
//TEST:SIMPLE_EX(filecheck=WGSL_NO_INT_CAST): -lang glsl tests/glsl-intrinsic/glsl-sign-float-type.slang -stage compute -entry computeMain -target wgsl

// --- Runtime check (Vulkan) ---

//TEST:COMPARE_COMPUTE(targets=vk, filecheck-buffer=BUF):-entry computeMain -source-language glsl -output-using-type

#version 450
layout(local_size_x = 1) in;

// Input: -3.0, 3.0, -1.0 stored as float bit patterns.
//TEST_INPUT:ubuffer(data=[-3.0 3.0 -1.0], stride=4):name=inputBuffer
layout(binding = 0) buffer InputBlock { float data[]; } inputBuffer;

//TEST_INPUT:ubuffer(data=[0.0 0.0 0.0 0.0 0.0], stride=4):out,name=outputBuffer
layout(binding = 1) buffer OutputBlock { float data[]; } outputBuffer;

void computeMain()
{
    float nx = inputBuffer.data[0]; // -3.0
    float px = inputBuffer.data[1]; //  3.0
    float nz = inputBuffer.data[2]; // -1.0

    // --- scalar float ---
    // sign(float) must produce a float result via FSign; there must be no
    // OpConvertFToS (cast to int) wrapping the result, and division must use
    // FDiv not SDiv.
    //
    // SPIR: FSign
    // SPIR-NOT: OpConvertFToS
    outputBuffer.data[0] = sign(nx) / 2.0;    // expected: -0.5
    outputBuffer.data[1] = sign(px) / 2.0;    // expected:  0.5

    // --- vec2 float ---
    // The vector overload must also stay in float: FSign %v2float, not
    // FSign %v2float + ConvertFToS %v2int.
    //
    // SPIR: FSign
    // SPIR-NOT: OpConvertFToS
    vec2 v = vec2(nx, px);
    vec2 sv = sign(v) / 2.0;
    outputBuffer.data[2] = sv.x;              // expected: -0.5
    outputBuffer.data[3] = sv.y;              // expected:  0.5

    // --- scalar float, division by integer literal ---
    // The original bug report used `sign(x) / 2` (integer literal). After the
    // fix, the integer literal must be promoted to float so the division
    // remains a float operation.
    //
    // SPIR: FSign
    // SPIR-NOT: OpSDiv
    outputBuffer.data[4] = sign(nx) / 2;   // expected: -0.5 (nx = -3.0)

    // GLSL output must not emit the int(sign(...)) wrapper that the old
    // code produced for float sign().
    // GLSL-NOT: int(sign(
    // GLSL: sign(

    // WGSL output must emit native sign()
    // WGSL: sign(

    // BUF: BF000000
    // BUF-NEXT: 3F000000
    // BUF-NEXT: BF000000
    // BUF-NEXT: 3F000000
    // BUF-NEXT: BF000000
}

// Global absence check: after the fix no float→int conversion should appear
// anywhere in the SPIR-V produced for a pure-float sign() shader.
// SPIR_NO_INT_CAST-NOT: OpConvertFToS
// SPIR_NO_INT_CAST-NOT: OpSDiv

// Global absence check (WGSL): ensure no integer wrapper surrounds sign() calls anywhere in WGSL output.
// WGSL_NO_INT_CAST-NOT: i32(sign(
// WGSL_NO_INT_CAST-NOT: vec2<i32>(sign(
