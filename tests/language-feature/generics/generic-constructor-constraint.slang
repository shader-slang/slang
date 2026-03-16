// generic-constructor-constraint.slang

// Test that a generic constructor with interface constraints works correctly.
// The struct has a generic __init that requires T to conform to IArithmetic,
// and the constructed object's methods produce correct results.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK): -output-using-type

//TEST_INPUT: set output = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<int> output;

interface IValueProvider
{
    int getValue();
}

struct IntProvider : IValueProvider
{
    int v;
    int getValue() { return v; }
}

struct FloatProvider : IValueProvider
{
    float v;
    int getValue() { return int(v); }
}

struct Container
{
    int stored;

    __init<T : IValueProvider>(T provider, int scale)
    {
        stored = provider.getValue() * scale;
    }

    int getStored() { return stored; }
}

[numthreads(1, 1, 1)]
void computeMain()
{
    IntProvider ip;
    ip.v = 5;
    Container c1 = Container(ip, 3);

    FloatProvider fp;
    fp.v = 2.5;
    Container c2 = Container(fp, 4);

    // 5 * 3 = 15
    // CHECK: 15
    output[0] = c1.getStored();

    // int(2.5) * 4 = 2 * 4 = 8
    // CHECK: 8
    output[1] = c2.getStored();

    // Sum
    // CHECK: 23
    output[2] = c1.getStored() + c2.getStored();
}
