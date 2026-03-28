//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type
//TEST:SIMPLE(filecheck=REPORT):-target hlsl -stage compute -entry computeMain -report-dynamic-dispatch-sites

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

// Test multiple reassignments of interface variables
// This verifies that the type tag is properly updated and
// the correct implementation is dispatched after each reassignment.

interface ICalculator
{
    float compute(float x);
}

struct Square : ICalculator
{
    float factor;
    float compute(float x) { return x * x * factor; }
};

struct Cube : ICalculator
{
    float factor;
    float compute(float x) { return x * x * x * factor; }
};

struct Linear : ICalculator
{
    float factor;
    float compute(float x) { return x * factor; }
};

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // Test 1: Basic reassignment between three types
    ICalculator calc;
    
    calc = Square(2.0);
    outputBuffer[0] = calc.compute(3.0); // CHECK: 18.0
    
    calc = Cube(1.0);
    outputBuffer[1] = calc.compute(3.0); // CHECK: 27.0
    
    calc = Linear(5.0);
    outputBuffer[2] = calc.compute(3.0); // CHECK: 15.0
    
    // Test 2: Reassignment in a loop
    float results[3] = {0, 0, 0};
    for (uint i = 0; i < 3; i++)
    {
        if (i == 0)
            calc = Square(1.0);
        else if (i == 1)
            calc = Cube(1.0);
        else
            calc = Linear(2.0);
        
        results[i] = calc.compute(2.0);
        // REPORT: ([[# @LINE-1]]): {{.*}} generated {{.*}} dispatch code{{.*}} 3 possible types:
    }
    
    outputBuffer[3] = results[0]; // CHECK: 4.0
    outputBuffer[4] = results[1]; // CHECK: 8.0
    outputBuffer[5] = results[2]; // CHECK: 4.0
    
    // Test 3: Reassignment after conditional use
    calc = Square(3.0);
    float temp1 = calc.compute(2.0); // 12.0
    
    calc = Cube(2.0);
    float temp2 = calc.compute(2.0); // 16.0
    
    outputBuffer[6] = temp1;         // CHECK: 12.0
    outputBuffer[7] = temp2;         // CHECK: 16.0
    
    // Test 4: Multiple reassignments in sequence
    calc = Linear(10.0);
    calc = Square(4.0);
    calc = Cube(0.5);
    outputBuffer[8] = calc.compute(4.0); // CHECK: 32.0
    
    // Test 5: Reassignment with same type but different data
    calc = Square(1.0);
    float val1 = calc.compute(5.0);      // 25.0
    
    calc = Square(2.0);
    float val2 = calc.compute(5.0);      // 50.0
    
    outputBuffer[9] = val1;              // CHECK: 25.0
    outputBuffer[10] = val2;             // CHECK: 50.0
}
