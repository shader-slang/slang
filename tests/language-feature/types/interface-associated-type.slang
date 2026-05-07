// interface-associated-type.slang

// Test associated types via `associatedtype` in interfaces.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type

interface IContainer
{
    associatedtype Element;
    Element getElement(int index);
    int getCount();
}

struct IntPair : IContainer
{
    typedef int Element;
    int a;
    int b;

    int getElement(int index)
    {
        if (index == 0) return a;
        return b;
    }

    int getCount() { return 2; }
}

int sumElements<T : IContainer>(T container)
    where T.Element == int
{
    int sum = 0;
    for (int i = 0; i < container.getCount(); i++)
    {
        int elem = container.getElement(i);
        sum = sum + elem;
    }
    return sum;
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1,1,1)]
void computeMain()
{
    IntPair pair;
    pair.a = 3;
    pair.b = 7;

    outputBuffer[0] = sumElements(pair);
    // CHECK: 10

    outputBuffer[1] = pair.getCount();
    // CHECK: 2
}
