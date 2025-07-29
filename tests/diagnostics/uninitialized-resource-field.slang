//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -stage compute -entry computeMain -target hlsl

RWStructuredBuffer<int> outputBuffer;

void useStruct(SomeStruct s)
{
    outputBuffer[0] = s.value;
}

[shader("compute")]
[numthreads(1,1,1)]
void computeMain(uint3 threadId : SV_DispatchThreadID)
{
    // CHECK: ([[# @LINE+1]]): error {{.*}} cannot default-initialize struct 'SomeStruct' with '{}' because it contains an uninitialized texture field
    SomeStruct s = {};

    s.value = 10 + threadId.x;
    useStruct(s);
}

// CHECK: ([[# @LINE+1]]): note: see definition of struct 'SomeStruct'
struct SomeStruct
{
    int value;
    Texture1D<float> tex;
}
