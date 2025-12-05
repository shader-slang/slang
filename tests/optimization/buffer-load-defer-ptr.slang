//TEST:SIMPLE(filecheck=SPV): -target spirv

struct Bottom
{
    float bigArray[1024];
    float bottomGetValue(int index) { return bigArray[index]; }
}

struct Middle
{
    Bottom bottom;
    float middleGetValue(int index) { return bottom.bottomGetValue(index); }
}

struct Top
{
    Middle middle;
    float topGetValue(int index) { return middle.middleGetValue(index); }
}

struct Root
{
    Top top;
}

uniform ImmutablePtr<Root> cb;

RWStructuredBuffer<float> outputBuffer;

// SPV: OpEntryPoint
// SPV-NOT: OpCompositeConstruct

[shader("compute")]
[numthreads(1, 1, 1)]
void compute_main(uint3 tid: SV_DispatchThreadID)
{
    outputBuffer[0] = cb.top.topGetValue(0);
}
