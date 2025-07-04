//TEST:SIMPLE(filecheck=WGSL): -stage compute -entry computeMain -target wgsl

// Test that switch case values are emitted without type casts in WGSL

struct TestData
{
    uint intValue;
    float floatValue;
    int signedValue;
}

RWStructuredBuffer<TestData> dataBuffer;

// Function with integer switch cases
// WGSL: case 0:
// WGSL: case 1:
// WGSL: case 2:
uint processIntSwitch(uint value)
{
    switch (value)
    {
        case 0: return 10;
        case 1: return 20;
        case 2: return 30;
        default: return 0;
    }
}

// Function with signed integer switch cases
// WGSL: case -1:
// WGSL: case 0:
// WGSL: case 1:
int processSignedSwitch(int value)
{
    switch (value)
    {
        case -1: return -10;
        case 0: return 0;
        case 1: return 10;
        default: return 999;
    }
}

// Function with multiple case values on same branch
// WGSL: case 10, 20, 30:
uint processMultipleCase(uint value)
{
    switch (value)
    {
        case 10:
        case 20:
        case 30:
            return 100;
        default:
            return 0;
    }
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    TestData data = dataBuffer[0];
    
    // Test different switch patterns
    uint result1 = processIntSwitch(data.intValue);
    int result2 = processSignedSwitch(data.signedValue);
    uint result3 = processMultipleCase(data.intValue);
    
    data.intValue = result1;
    data.signedValue = result2;
    data.floatValue = float(result3);
    dataBuffer[0] = data;
}