// Test tuple swizzling and element access.

#language 2026

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-dx12 -use-dxil -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -shaderobj

//TEST_INPUT:ubuffer(data=[1 2 3 4], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

void store(inout Tuple<int, float, uint> t)
{
    t._1_2 = (3.0, 4u);
}

struct MyType
{
    int x;
    int y;
    __init(Tuple<int, int> t){
        x = t._0;
        y = t._1;
    }
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    var t = (1, 2.0, 3u);
    store(t);
    let y = t._1_2_0;

    // CHECK: 4
    outputBuffer[0] = y._1;

    // This should mean cast<MyType>(makeTuple(1,2))
    let m = (MyType)(1,2);
    // CHECK: 3
    outputBuffer[1] = m.x + m.y;
}
