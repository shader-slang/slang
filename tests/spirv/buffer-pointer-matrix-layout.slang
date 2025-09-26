//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk -output-using-type -xslang -matrix-layout-column-major -emit-spirv-directly

// TEST_INPUT: set ptr = ubuffer(data=[1.0 2.0 3.0 4.0 5.0 6.0 7.0 8.0 9.0 10.0 11.0 12.0],stride=4)
uniform float3x4 *ptr;

// TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0],stride=4)
RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 dtid: SV_DispatchThreadID)
{
    // This matrix is in memry column major. Slang respects this here and load it properly!
    float3x4 correctly_read_matrix = *ptr;
    outputBuffer[0] = correctly_read_matrix[0][0];
    outputBuffer[1] = correctly_read_matrix[0][1];
    outputBuffer[2] = correctly_read_matrix[0][2];
    outputBuffer[3] = correctly_read_matrix[0][3];
    outputBuffer[4] = correctly_read_matrix[1][0];
    outputBuffer[5] = correctly_read_matrix[1][1];
    outputBuffer[6] = correctly_read_matrix[1][2];
    outputBuffer[7] = correctly_read_matrix[1][3];
    // CHECK: 1.0
    // CHECK: 4.0
    // CHECK: 7.0
    // CHECK: 10.0
    // CHECK: 2.0
    // CHECK: 5.0
    // CHECK: 8.0
    // CHECK: 11.0

    // With this syntax however, Slang was ignoring the column major setting and loads it as it it was row major!
    float3x4 broken_matrix = ptr[0];
    outputBuffer[8] = broken_matrix[0][0];
    outputBuffer[9] = broken_matrix[0][1];
    outputBuffer[10] = broken_matrix[0][2];
    outputBuffer[11] = broken_matrix[0][3];
    outputBuffer[12] = broken_matrix[1][0];
    outputBuffer[13] = broken_matrix[1][1];
    outputBuffer[14] = broken_matrix[1][2];
    outputBuffer[15] = broken_matrix[1][3];
    // CHECK: 1.0
    // CHECK: 4.0
    // CHECK: 7.0
    // CHECK: 10.0
    // CHECK: 2.0
    // CHECK: 5.0
    // CHECK: 8.0
    // CHECK: 11.0
}