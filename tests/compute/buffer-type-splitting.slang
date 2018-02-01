//TEST(compute):COMPARE_COMPUTE:
//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):dxbinding(0),glbinding(0),out
//TEST_INPUT:ubuffer(data=[0 2 3 3]):dxbinding(1),glbinding(1)
//TEST_INPUT:ubuffer(data=[4 5 6 7]):dxbinding(2),glbinding(2)
//TEST_INPUT:ubuffer(data=[8 9 10 11]):dxbinding(3),glbinding(3)
//TEST_INPUT:ubuffer(data=[12 13 14 15]):dxbinding(4),glbinding(4)

RWStructuredBuffer<int> outputBuffer;

struct S
{
	RWByteAddressBuffer a;
	RWByteAddressBuffer b;
};
S s[2];

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint i = dispatchThreadID.x;

    int val =
          s[0].a.Load(i * 4)
        + s[1].a.Load(i * 4)*16
        + s[0].b.Load(i * 4)*256
        + s[1].b.Load(i * 4)*4096;

    outputBuffer[i] = val; 
}