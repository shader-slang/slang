//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):-target hlsl -stage compute -entry computeMain -profile cs_6_7

// Test that SM 6.7 basic compilation works

//TEST_INPUT: ubuffer(data=[0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<uint4> outputBuffer;

void computeMain()
{
    uint4 testResult = uint4(1,2,3,4);
    outputBuffer[0] = testResult;
    // CHECK: uint4
}