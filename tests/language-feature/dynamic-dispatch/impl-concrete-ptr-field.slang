// Interface implementation with a concrete-type pointer field. The pointer
// field is packed into AnyValue as raw bits (bitcast to uint/uint2) during
// dynamic dispatch. After unpacking, the pointer must still be valid and
// dereferenceable.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//DISABLE_TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type -emit-spirv-directly
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

struct Data
{
    int x;
    int y;
};

interface IProcessor
{
    int process();
}

struct PtrProcessor : IProcessor
{
    Data* data;
    int process()
    {
        return data->x + data->y;
    }
}

struct SimpleProcessor : IProcessor
{
    int value;
    int process()
    {
        return value * 2;
    }
}

IProcessor makeProcessor(int id, Data* pData, int simpleVal)
{
    if (id == 0)
    {
        PtrProcessor p;
        p.data = pData;
        return p;
    }
    else
    {
        SimpleProcessor s;
        s.value = simpleVal;
        return s;
    }
}

//TEST_INPUT:ubuffer(data=[0 1], stride=4):name=typeSelector
RWStructuredBuffer<uint> typeSelector;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    Data d;
    d.x = 30;
    d.y = 12;

    // PtrProcessor path: dereferences Data* and returns x + y = 42
    var proc = makeProcessor(typeSelector[0], &d, 0);
    // CHECK: 42
    outputBuffer[0] = proc.process();

    // SimpleProcessor path: returns value * 2 = 50
    var proc2 = makeProcessor(typeSelector[1], nullptr, 25);
    // CHECK: 50
    outputBuffer[1] = proc2.process();

    // Modify the data and re-dispatch through PtrProcessor
    d.x = 100;
    d.y = -10;
    var proc3 = makeProcessor(typeSelector[0], &d, 0);
    // CHECK: 90
    outputBuffer[2] = proc3.process();
}
