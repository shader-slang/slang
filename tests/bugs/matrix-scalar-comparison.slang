// Comparing a matrix with a scalar compares the scalar with every element. On the targets that
// lower bool and integer matrices to arrays of vectors (SPIR-V, GLSL, Metal, WGSL), the bool
// matrix result is lowered by `legalizeComparisonOperation`, which used to assert that both
// operands were matrices and crash slangc. `clip` on a matrix hits the same path; see
// matrix-scalar-comparison-clip.slang.

//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=GLSL): -target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METAL): -target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSL): -target wgsl -entry computeMain -stage compute
// slang-test also synthesizes a CUDA lane from the -cpu lane.
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-cpu -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-metal -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-wgpu -output-using-type

// SPIRV-NOT: {{error\[E[0-9]+\]}}
// SPIRV: OpFOrdGreaterThan
// SPIRV: OpFOrdEqual
// SPIRV: OpSLessThan
// GLSL-NOT: {{error\[E[0-9]+\]}}
// GLSL: greaterThan
// METAL-NOT: {{error\[E[0-9]+\]}}
// METAL: computeMain
// WGSL-NOT: {{error\[E[0-9]+\]}}
// WGSL: computeMain

//TEST_INPUT:ubuffer(data=[0.5 -1.0 2.0 3.0], stride=4):name=inputBuffer
RWStructuredBuffer<float> inputBuffer;

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

// One bit per element, in row-major order, so a misplaced row or column changes the value.
int bits(bool2x2 b)
{
    return int(b[0][0]) | (int(b[0][1]) << 1) | (int(b[1][0]) << 2) | (int(b[1][1]) << 3);
}

[numthreads(1, 1, 1)]
void computeMain()
{
    float2x2 m = float2x2(inputBuffer[0], inputBuffer[1], inputBuffer[2], inputBuffer[3]);
    int3x2 n = int3x2(int(inputBuffer[3]));
    n[1][0] = -2;

    // m is (0.5, -1; 2, 3).
    // Matrix on the left: every element but m[0][1] = -1 is greater than 0 (bits 0, 2, 3).
    // BUF: type: int32_t
    // BUF-NEXT: 13
    outputBuffer[0] = bits(m > 0.0);

    // Scalar on the left: only m[1][0] is 2 (bit 2).
    // BUF-NEXT: 4
    outputBuffer[1] = bits(2.0 == m);

    // An integer matrix, which is lowered itself, against a scalar: only n[1][0] = -2 is
    // negative (row 1, column 0, so 1 << (1 * 2 + 0)).
    // BUF-NEXT: 4
    bool3x2 negative = n < 0;
    int negativeBits = 0;
    for (int r = 0; r < 3; r++)
        negativeBits |= (int(negative[r][0]) | (int(negative[r][1]) << 1)) << (r * 2);
    outputBuffer[2] = negativeBits;

    // BUF-NEXT: 1
    outputBuffer[3] = (all(m > -2.0) && !any(m > 5.0)) ? 1 : 0;
}
