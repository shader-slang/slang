//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-dx12 -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-mtl -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-wgsl -output-using-type

// Test for dot product with 1-element vectors (float and int)

// CHECK: 8

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // Float dot product with 1-element vectors
    vector<float, 1> floatVec1 = vector<float, 1>(2.0);
    vector<float, 1> floatVec2 = vector<float, 1>(2.0);
    float floatDot = dot(floatVec1, floatVec2); // 2.0 * 2.0 = 4.0
    
    // Int dot product with 1-element vectors
    vector<int, 1> intVec1 = vector<int, 1>(2);
    vector<int, 1> intVec2 = vector<int, 1>(2);
    int intDot = dot(intVec1, intVec2); // 2 * 2 = 4
    
    // Add them together and convert to int
    int result = int(floatDot) + intDot; // 4 + 4 = 8
    
    outputBuffer[0] = result;
}
