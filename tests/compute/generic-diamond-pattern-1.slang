//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type

// Confirm that generics syntax can be used in user
// code and generates valid output.

//TEST_INPUT:ubuffer(data=[0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

interface Top
{

};

interface MidLeft : Top
{

};

interface MidRight : Top
{

};

interface Bottom : MidLeft, MidRight
{

};

struct GenFoo<T : Top>
{
    T x;
};

T testRight<T : MidRight>(GenFoo<T> val)
{
    return val.x;
}

T testLeft<T : MidLeft>(GenFoo<T> val)
{
    return val.x;
}

struct BottomImpl : Bottom
{
    float x;
};


[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint tid = dispatchThreadID.x;

    {
        float inVal = float(tid);
        GenFoo<BottomImpl> outVal = { {inVal} };
        BottomImpl bi = testRight(outVal);
        outputBuffer[tid] = bi.x;
        
        // CHECK: 0.0
        // CHECK-NEXT: 1.0
        // CHECK-NEXT: 2.0
        // CHECK-NEXT: 3.0
    }

    {
        float inVal = float(tid);
        GenFoo<BottomImpl> outVal = { {inVal} };
        BottomImpl bi = testLeft(outVal);
        outputBuffer[tid + 4] = bi.x;
        
        // CHECK-NEXT: 0.0
        // CHECK-NEXT: 1.0
        // CHECK-NEXT: 2.0
        // CHECK-NEXT: 3.0
    }

}