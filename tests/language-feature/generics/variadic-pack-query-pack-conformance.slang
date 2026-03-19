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

int trimFirstExplicit<each U : IBar>(expand each U values) where nonempty(U)
{
    acceptPack<__trimFirst(U)>();
    return sumBars<__trimFirst(U)>(expand each __trimFirst(values));
}

int trimFirstInferred<each U : IBar>(expand each U values) where nonempty(U)
{
    return sumBars(expand each __trimFirst(values));
}

int trimLastExplicit<each U : IBar>(expand each U values) where nonempty(U)
{
    acceptPack<__trimLast(U)>();
    return sumBars<__trimLast(U)>(expand each __trimLast(values));
}

int trimLastInferred<each U : IBar>(expand each U values) where nonempty(U)
{
    return sumBars(expand each __trimLast(values));
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
    outputBuffer[0] = trimFirstExplicit(BarA(2), BarB(3), BarC(5));

    // CHECK: 8
    outputBuffer[1] = trimFirstInferred(BarA(2), BarB(3), BarC(5));

    // CHECK: 5
    outputBuffer[2] = trimLastExplicit(BarA(2), BarB(3), BarC(5));

    // CHECK: 5
    outputBuffer[3] = trimLastInferred(BarA(2), BarB(3), BarC(5));

    // CHECK: 10
    outputBuffer[4] = expandExplicit(BarA(2), BarB(3), BarC(5));
}
