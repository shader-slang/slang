//TEST(compute):COMPARE_COMPUTE:-xslang -use-ir
//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):dxbinding(0),glbinding(0),out
// Confirm that generics syntax can be used in user
// code and generates valid output.

RWStructuredBuffer<float4> outputBuffer;

interface IElement
{
    float4 getValue();
};

struct SingleElement : IElement
{
    float4 value;
    float4 getValue()
    {
        return value;
    }
};

struct ListElement<THead : IElement> : IElement
{
    THead head;
    float4 getValue()
    {
        return head.getValue();
    }
};

float4 test<T : IElement>(T val)
{
	return val.getValue();
}


[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    ListElement<SingleElement> list;
    list.head.value = float4(1.0);
    float4 outVal = test<ListElement<SingleElement> >(list);
	outputBuffer[0] = outVal;
}