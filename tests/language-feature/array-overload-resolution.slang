//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-output-using-type -cpu

// Test that sized arrays can be passed to unsized array parameters
// during function overload resolution.

float4 Function(float4[] f) { return f[0]; }
float4 Function(RWStructuredBuffer<float4> f) { return f[0]; }

// Test with different array sizes
int TestFunc(int[] arr) { return arr[0]; }
int TestFunc(float val) { return (int)val; }

RWStructuredBuffer<float4> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    static float4[16] input1 = { 
        float4(1, 2, 3, 4), 
        float4(5, 6, 7, 8)
    };
    static float4[8] input2 = { 
        float4(10, 11, 12, 13) 
    };
    
    // Test int arrays too
    static int[5] intArray1 = { 42 };
    static int[3] intArray2 = { 100 };
    
    float4 result1 = Function(input1);  // Should call array version
    float4 result2 = Function(input2);  // Should call array version
    int intResult1 = TestFunc(intArray1);  // Should call array version
    int intResult2 = TestFunc(intArray2);  // Should call array version
    
    // Store results
    outputBuffer[0] = result1 + result2 + float4(intResult1, intResult2, 0, 0);
}

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=16):out
//CHECK: 113.000000