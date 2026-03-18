//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

interface IBar
{
    int getValue();
}

struct BarA : IBar
{
    int value;
    int getValue() { return value; }
}

struct BarB : IBar
{
    int value;
    int getValue() { return value; }
}

struct BarC : IBar
{
    int value;
    int getValue() { return value; }
}

struct Wrapper<T : IBar> : IBar
{
    T value;
    int getValue() { return value.getValue(); }
}

void acceptPack<each T : IBar>() {}

int sumBars<each T : IBar>(expand each T values)
{
    int sum = 0;
    expand(sum += (each values).getValue());
    return sum;
}

Wrapper<T> makeWrapper<T : IBar>(T value)
{
    Wrapper<T> result = {value};
    return result;
}

int trimHeadExplicit<each U : IBar>(expand each U values) where nonempty(U)
{
    acceptPack<__trimHead(U)>();
    return sumBars<__trimHead(U)>(expand each __trimHead(values));
}

int trimHeadInferred<each U : IBar>(expand each U values) where nonempty(U)
{
    return sumBars(expand each __trimHead(values));
}

int trimTailExplicit<each U : IBar>(expand each U values) where nonempty(U)
{
    acceptPack<__trimTail(U)>();
    return sumBars<__trimTail(U)>(expand each __trimTail(values));
}

int trimTailInferred<each U : IBar>(expand each U values) where nonempty(U)
{
    return sumBars(expand each __trimTail(values));
}

int expandExplicit<each U : IBar>(expand each U values)
{
    acceptPack<expand Wrapper<each U>>();
    return sumBars<expand Wrapper<each U>>(expand makeWrapper((each values)));
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    // CHECK: 8
    outputBuffer[0] = trimHeadExplicit(BarA(2), BarB(3), BarC(5));

    // CHECK: 8
    outputBuffer[1] = trimHeadInferred(BarA(2), BarB(3), BarC(5));

    // CHECK: 5
    outputBuffer[2] = trimTailExplicit(BarA(2), BarB(3), BarC(5));

    // CHECK: 5
    outputBuffer[3] = trimTailInferred(BarA(2), BarB(3), BarC(5));

    // CHECK: 10
    outputBuffer[4] = expandExplicit(BarA(2), BarB(3), BarC(5));
}
