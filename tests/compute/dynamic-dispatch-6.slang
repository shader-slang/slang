//TEST(compute):COMPARE_COMPUTE:-dx11 -shaderobj
//TEST(compute):COMPARE_COMPUTE:-vk -shaderobj
//TEST(compute):COMPARE_COMPUTE:-cpu -shaderobj
//TEST(compute):COMPARE_COMPUTE:-cuda -shaderobj
//TEST(compute):COMPARE_COMPUTE:-cuda -shaderobj -capability cuda_sm_7_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3
//TEST:SIMPLE(filecheck=NVVM):-target ptx -emit-cuda-via-nvvm -entry computeMain -stage compute -capability cuda_sm_7_0 -O3 -o -

// NVVM: .visible .entry computeMain()
// NVVM: st.global.u32

// Test dynamic dispatch code gen for generic-typed return values.
[anyValueSize(8)]
interface IInterface
{
    [mutating]
    void SetVal(int inVal);
    int Compute();
};

T CreateT_Inner<T:IInterface>(int inVal)
{
    T obj;
    obj.SetVal(inVal);
    return obj;
}

T CreateT<T:IInterface>(int inVal)
{
    return CreateT_Inner<T>(inVal);
}

struct Impl : IInterface
{
    int base;
    [mutating]
    void SetVal(int inVal)
    {
        base = inVal;
    }
    int Compute()
    {
        return base;
    }
};

int test()
{
    var obj = CreateT<Impl>(3);
    var obj2 = CreateT_Inner<Impl>(1);
    return obj.Compute() + obj2.Compute();
}

//TEST_INPUT:ubuffer(data=[0 1 2 3], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint tid = dispatchThreadID.x;
    int outVal = test();
    outputBuffer[tid] = outVal;
}
