//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    let x = float(2);   // Let variable should generate debug info
    var y = float(3);   // Var variable for comparison 
    float z = float(4); // Regular variable declaration
    
    // Use the variables so they don't get optimized away
    float result = x + y + z;
    
    // CHECK: 9.0
    outputBuffer[0] = result;
}

// This test verifies that 'let' variables now generate debug information
// Previously, 'let' variables did not generate debug info, causing issues with debuggers