//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUFFER):-shaderobj -vk
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUFFER):-shaderobj

struct ExplicitDefault
{
    int value;

    __init(int value = 0)
    {
        this.value = value;
    }
}

struct UsesLaterType
{
    LaterType later;
}

struct LaterType
{
    ExplicitDefault leaf;
}

struct Outer
{
    UsesLaterType data;
}

//TEST_INPUT:ubuffer(data=[1], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

void init(inout Outer o)
{
    o = {};
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    Outer o;
    init(o);

    // BUFFER: 0
    outputBuffer[0] = o.data.later.leaf.value;
}
