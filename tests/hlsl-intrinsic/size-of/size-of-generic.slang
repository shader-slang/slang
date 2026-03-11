
//TEST(compute):COMPARE_COMPUTE_EX:-cpu -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -dx12 -shaderobj
//TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE_EX:-cuda -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE_EX:-llvm -compute -shaderobj

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0], stride=4):out,name outputBuffer

RWStructuredBuffer<int> outputBuffer;

struct Thing<T>
{
    T t;
};

int getVecSize<T>()
{
    return sizeof(vector<T, 4>);
}

int getAtomicSize<T : IAtomicable>()
{
    return sizeof(Atomic<T>);
}

[numthreads(6, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    const int idx = asint(dispatchThreadID.x);

    int size = 0;
    
    switch (idx)
    {   
        case 0:     size = sizeof(Thing<float>); break;
        case 1:     size = sizeof(Thing<float3>); break;
        case 2:     size = sizeof(Thing<matrix<uint, 3, 2>>); break;
        case 3:     size = getVecSize<float>(); break;
        case 4:     size = getAtomicSize<int>(); break;
        case 5:     size = getAtomicSize<uint64_t>(); break;
        default: break;
    }

    outputBuffer[idx] = size;
}
