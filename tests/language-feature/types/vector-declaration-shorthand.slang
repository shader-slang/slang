// vector-declaration-shorthand.slang

// Test that vector declaration using generic syntax (vector<float,3>) and
// shorthand (float3) are interchangeable.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // Declare using generic syntax
    vector<float, 3> a = vector<float, 3>(1.0, 2.0, 3.0);

    // Declare using shorthand syntax
    float3 b = float3(1.0, 2.0, 3.0);

    // Both should be identical — sum of elements should match
    // CHECK: 6.0
    outputBuffer[0] = a.x + a.y + a.z;

    // CHECK: 6.0
    outputBuffer[1] = b.x + b.y + b.z;

    // Assign generic to shorthand and vice versa (interchangeable)
    float3 c = a;
    vector<float, 3> d = b;

    // CHECK: 1.0
    outputBuffer[2] = c.x;

    // CHECK: 2.0
    outputBuffer[3] = d.y;

    // vector<int, 2> and int2 are interchangeable
    vector<int, 2> ei = vector<int, 2>(5, 7);
    int2 fi = int2(5, 7);

    // CHECK: 12.0
    outputBuffer[4] = float(ei.x + ei.y);

    // CHECK: 12.0
    outputBuffer[5] = float(fi.x + fi.y);
}
