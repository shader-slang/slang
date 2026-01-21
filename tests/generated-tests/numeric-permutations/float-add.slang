//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

// Test: float+ operator

[numthreads(1,1,1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    float a = 10.0;
    float b = 5;
    float result = a + b;
    outputBuffer[0] = result;
}

// CHECK: 15
