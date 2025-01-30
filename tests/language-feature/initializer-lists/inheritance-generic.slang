//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUFFER):-shaderobj -vk
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUFFER):-shaderobj

struct Base<let ND:int>
{
    int a = 1;
}

struct Derived<let ND:int>: Base<ND>
{
    bool x;
    bool y;
}

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=result
RWStructuredBuffer<int> result;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    Derived<3> d;

    // BUFFER: 1
    result[0] = d.a;
}
