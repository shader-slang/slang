// Large vectors and matrices in buffers are laid out as arrays: a vector<float, 5> takes 5
// consecutive floats (no padding to a 4-element boundary), and a matrix<float, 3, 6> takes 3 rows
// of 6 floats one after the other (row-major, whatever the matrix layout mode).

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -emit-spirv-via-glsl
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-dx12 -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-metal -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-wgpu -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type

struct Element
{
    vector<float, 5> v;
    float tag;
    matrix<float, 3, 6> m;
}

// Two elements of 5 + 1 + 18 = 24 floats each.
//TEST_INPUT:ubuffer(data=[1.0 2.0 3.0 4.0 5.0 6.0 10.0 11.0 12.0 13.0 14.0 15.0 16.0 17.0 18.0 19.0 20.0 21.0 22.0 23.0 24.0 25.0 26.0 27.0 1.5 2.5 3.5 4.5 5.5 6.5 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0], stride=96):name=inputBuffer
StructuredBuffer<Element> inputBuffer;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=96):out,name=copyBuffer
RWStructuredBuffer<Element> copyBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    Element e = inputBuffer[0];
    // CHECK: {{^}}5.000000
    outputBuffer[0] = e.v[4];
    // CHECK-NEXT: {{^}}6.000000
    outputBuffer[1] = e.tag;
    // CHECK-NEXT: {{^}}10.000000
    outputBuffer[2] = e.m[0][0];
    // CHECK-NEXT: {{^}}16.000000
    outputBuffer[3] = e.m[1][0];
    // CHECK-NEXT: {{^}}27.000000
    outputBuffer[4] = e.m[2][5];
    // CHECK-NEXT: {{^}}6.500000
    outputBuffer[5] = inputBuffer[1].tag;

    // Write a whole element back, then read it again through the buffer.
    e.m[2][5] = e.v[0] + e.tag;
    copyBuffer[0] = e;
    // CHECK-NEXT: {{^}}7.000000
    outputBuffer[6] = copyBuffer[0].m[2][5];
    // CHECK-NEXT: {{^}}4.000000
    outputBuffer[7] = copyBuffer[0].v.w;

    // Element-wise operations directly on loaded values (the row type appears only while
    // splitting the matrices).
    // CHECK-NEXT: {{^}}27.000000
    outputBuffer[8] = (inputBuffer[0].m + inputBuffer[1].m)[2][5];
    // CHECK-NEXT: {{^}}5.000000
    outputBuffer[9] = (inputBuffer[0].v * 2.0 - inputBuffer[1].v)[1] + 3.5;

    // The copied element, as raw bits: v = 1..5, tag = 6, m = 10..26 then 7 (the updated m[2][5]).
    // CHECK: {{^}}3F800000
    // CHECK-NEXT: {{^}}40000000
    // CHECK-NEXT: {{^}}40400000
    // CHECK-NEXT: {{^}}40800000
    // CHECK-NEXT: {{^}}40A00000
    // CHECK-NEXT: {{^}}40C00000
    // CHECK-NEXT: {{^}}41200000
    // CHECK: {{^}}41D00000
    // CHECK-NEXT: {{^}}40E00000
}
