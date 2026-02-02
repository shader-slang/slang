// Test using interface typed shader parameters with texture typed fields.
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -emit-spirv-directly -render-features fp8
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type

[anyValueSize(4)]
interface IFoo
{
    float calc();
}

struct A : IFoo
{
    FloatE4M3 value0; // 1.0
    FloatE5M2 value1; // 2.0
    float calc()
    {
        return float(value0) - float(value1);
    }
}

struct B : IFoo
{
    vector<BFloat16, 2> data; // (3.0, 4.0)
    float calc()
    {
        return float(data.x) / float(data.y);
    }
}

//TEST_INPUT: set gOutputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<float> gOutputBuffer;
//TEST_INPUT: set inputBuffer = ubuffer(data=[0x4038 0x40804040 0 0], stride=4)
RWStructuredBuffer<uint> inputBuffer;

//TEST_INPUT: type_conformance A:IFoo=0
//TEST_INPUT: type_conformance B:IFoo=1

[numthreads(2, 1, 1)]
void computeMain(int3 dispatchThreadID : SV_DispatchThreadID)
{
    IFoo obj0 = createDynamicObject<IFoo>(dispatchThreadID.x, inputBuffer[dispatchThreadID.x]);
    // CHECK: -1.0
    // CHECK: 0.75
    gOutputBuffer[dispatchThreadID.x] = obj0.calc();
}
