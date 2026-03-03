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
    // CHECK: ([[# @LINE+1]]): error {{.*}} not enough arguments to call (got 0, expected 2)
    SomeStruct s = {};

    s.value = 10 + threadId.x;
    useStruct(s);
}

// CHECK: ([[# @LINE+1]]): note {{.*}}: candidate: SomeStruct.init(int, Texture1D<float>)
struct SomeStruct
{
    int value;
    Texture1D<float> tex;
}
