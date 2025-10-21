//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-dx12 -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-mtl -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-wgsl -output-using-type

// Test for dot product with 1-element vectors called from a generic function

// CHECK: 20

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

// Generic function that computes dot product for N-sized float vectors
__generic<let N : int>
float genericDotFloat(vector<float, N> a, vector<float, N> b)
{
    return dot(a, b);
}

// Generic function that computes dot product for N-sized int vectors
__generic<let N : int>
int genericDotInt(vector<int, N> a, vector<int, N> b)
{
    return dot(a, b);
}

// Generic function for testing with different N values
__generic<let N : int>
float testFloatDot(float value)
{
    vector<float, N> vec1;
    vector<float, N> vec2;
    
    // Initialize all components to the same value
    for (int i = 0; i < N; i++)
    {
        vec1[i] = value;
        vec2[i] = value;
    }
    
    return genericDotFloat(vec1, vec2);
}

// Generic function for testing integer dot products
__generic<let N : int>
int testIntDot(int value)
{
    vector<int, N> vec1;
    vector<int, N> vec2;
    
    // Initialize all components to the same value
    for (int i = 0; i < N; i++)
    {
        vec1[i] = value;
        vec2[i] = value;
    }
    
    return genericDotInt(vec1, vec2);
}

[numthreads(1, 1, 1)]
void computeMain()
{
    // Test with N=1 (single element vectors) - this is the main test case
    float floatResult1 = testFloatDot<1>(3.0);  // 3.0 * 3.0 = 9.0
    int intResult1 = testIntDot<1>(3);          // 3 * 3 = 9
    
    // Test with N=2 to ensure generic function works for other sizes
    float floatResult2 = testFloatDot<2>(1.0);  // (1.0*1.0 + 1.0*1.0) = 2.0
    
    // Sum all results: 9 + 9 + 2 = 20
    int result = int(floatResult1) + intResult1 + int(floatResult2);
    
    outputBuffer[0] = result;
}
