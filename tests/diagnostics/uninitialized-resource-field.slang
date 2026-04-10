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
    // CHECK: error{{.*}}E39999{{.*}}not enough arguments to call
    SomeStruct s = {};

    s.value = 10 + threadId.x;
    useStruct(s);
}

// CHECK: note{{.*}}E40011{{.*}}candidate: SomeStruct.init(int, Texture1D<float>)
struct SomeStruct
{
    int value;
    Texture1D<float> tex;
}
