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

[numthreads(1, 1, 1)]
void computeMain()
{
    float2x2 m = float2x2(inputBuffer[0], inputBuffer[1], inputBuffer[2], inputBuffer[3]);
    int3x2 n = int3x2(int(inputBuffer[3]));
    n[1][0] = -2;

    // Matrix on the left: 0.5, 2 and 3 are greater than 0; -1 is not.
    // BUF: type: int32_t
    // BUF-NEXT: 3
    bool2x2 positive = m > 0.0;
    outputBuffer[0] = int(positive[0][0]) + int(positive[0][1]) + int(positive[1][0]) + int(positive[1][1]);

    // Scalar on the left.
    // BUF-NEXT: 1
    bool2x2 isTwo = 2.0 == m;
    outputBuffer[1] = int(isTwo[0][0]) + int(isTwo[0][1]) + int(isTwo[1][0]) + int(isTwo[1][1]);

    // An integer matrix, which is lowered itself, against a scalar.
    // BUF-NEXT: 1
    bool3x2 negative = n < 0;
    outputBuffer[2] = int(any(negative[0])) + int(any(negative[1])) + int(any(negative[2]));

    // BUF-NEXT: 1
    outputBuffer[3] = (all(m > -2.0) && !any(m > 5.0)) ? 1 : 0;
}
