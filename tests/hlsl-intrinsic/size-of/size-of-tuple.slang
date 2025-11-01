//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-slang -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-slang -compute -dx12 -shaderobj
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-llvm -compute -shaderobj

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0], stride=4):out,name outputBuffer

RWStructuredBuffer<int> outputBuffer;

struct Thing<each T>
{
    int val;
    Tuple<expand each T> tuple;
};

int tupleSize1<T>(T vals)
{
    return sizeof(Tuple<T>);
}

int tupleSize2<each T>(T vals)
{
    return sizeof(Tuple<expand each T>);
}

int tupleSize3<each T>(T vals)
{
    return sizeof(Tuple<int, expand each T>);
}

int tupleSize4<each T>(T vals)
{
    return sizeof(Thing<expand each T>);
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    outputBuffer[0] = tupleSize2(); // CHECK: 0
    outputBuffer[1] = tupleSize3(); // CHECK-NEXT: 4
    outputBuffer[2] = tupleSize4(); // CHECK-NEXT: 4

    outputBuffer[3] = tupleSize1(1); // CHECK-NEXT: 4
    outputBuffer[4] = tupleSize2(1,2); // CHECK-NEXT: 8
    outputBuffer[5] = tupleSize3(1,2,3); // CHECK-NEXT: 10
    outputBuffer[6] = tupleSize4(1,2,3,4); // CHECK-NEXT: 14
}
