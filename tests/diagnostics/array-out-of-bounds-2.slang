// array-out-of-bounds-2.slang

// Test the specific scenario from the issue (array of size 3, accessing index 3)

//TEST:SIMPLE(filecheck=CHECK): -target spirv -entry computeMain -stage compute

struct SH3_t
{
    float coeffs[4];
};

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    SH3_t a[3];
    SH3_t b;
    
    for (int i = 0; i < 3; ++i)
    {
        // This should be fine
        outputBuffer[i] = a[i].coeffs[0];
    }
    
    // This reproduces the issue - accessing index 3 in array of size 3
    //CHECK: error 30029: array index '3' is out of bounds for array of size '3'.
    outputBuffer[3] = a[3].coeffs[0];
}