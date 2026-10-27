//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type

//TEST:SIMPLE(filecheck=DXIL): -target dxil-asm -entry computeMain -profile cs_6_0

// An HLSL method without a receiver specifier takes `this` as `inout`, so that it can modify a
// mutable receiver. As DXC does for methods that do not write `this`, Slang also accepts calls to
// such a method on an immutable receiver: a constant buffer member, a `StructuredBuffer` element,
// a constant, or a temporary. The call then receives a copy of the receiver, so the method's
// writes to `this` do not reach the immutable storage (issue #13964).

// DXIL: define void @computeMain()

import default_receiver_interface_lib;

struct Helper
{
    int value;

    int get() { return value; }

    int addAndGet(int delta)
    {
        value += delta;
        return value;
    }
};

// A default HLSL method also satisfies a requirement with an `in` receiver, such as those of the
// core module's `IComparable` and of the Slang interface `IShape`, through a synthesized wrapper
// that calls it on its immutable `this`.
struct Key : IComparable
{
    int key;

    bool lessThan(Key other) { return key < other.key; }
    bool lessThanOrEquals(Key other) { return key <= other.key; }
    bool equals(Key other) { return key == other.key; }
};

T minOf<T : IComparable>(T a, T b)
{
    return b.lessThan(a) ? b : a;
}

struct Square : IShape
{
    int side;

    int area() { return side * side; }
};

int areaOf<T : IShape>(T shape)
{
    return shape.area();
}

// The requirement of an interface written in HLSL has the HLSL default receiver, so a generic
// call through it also accepts an immutable receiver.
interface IValue
{
    int getValue();
};

struct Value : IValue
{
    int value;

    int getValue() { return value; }
};

int getValueOfConstant<T : IValue>(T value)
{
    const T constant = value;
    return constant.getValue();
}

Helper makeHelper(int value)
{
    Helper helper;
    helper.value = value;
    return helper;
}

int getThroughParam(Helper helper)
{
    return helper.get();
}

static const Helper kHelper = { 50 };

//TEST_INPUT:cbuffer(data=[10]):name C
cbuffer C
{
    Helper gHelper;
};

//TEST_INPUT: set gConstHelper = cbuffer(data=[20])
ConstantBuffer<Helper> gConstHelper;

//TEST_INPUT: set gHelpers = ubuffer(data=[30 31], stride=4)
StructuredBuffer<Helper> gHelpers;

//TEST_INPUT: set gMutableHelpers = ubuffer(data=[40], stride=4)
RWStructuredBuffer<Helper> gMutableHelpers;

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // CHECK: 10
    outputBuffer[0] = gHelper.get();
    // CHECK: 20
    outputBuffer[1] = gConstHelper.get();
    // CHECK: 31
    outputBuffer[2] = gHelpers[1].get();
    // CHECK: 50
    outputBuffer[3] = kHelper.get();
    // CHECK: 60
    outputBuffer[4] = makeHelper(60).get();
    const Helper localHelper = makeHelper(70);
    // CHECK: 70
    outputBuffer[5] = localHelper.get();
    // CHECK: 30
    outputBuffer[6] = getThroughParam(gHelpers[0]);

    // CHECK: 11
    outputBuffer[7] = gHelper.addAndGet(1);
    // CHECK: 10
    outputBuffer[8] = gHelper.get();
    // CHECK: 32
    outputBuffer[9] = gHelpers[0].addAndGet(2);
    // CHECK: 30
    outputBuffer[10] = gHelpers[0].get();

    // A mutable receiver observes the method's writes.
    gMutableHelpers[0].addAndGet(5);
    // CHECK: 45
    outputBuffer[11] = gMutableHelpers[0].get();

    Key a = { 4 };
    Key b = { 3 };
    // CHECK: 3
    outputBuffer[12] = minOf(a, b).key;
    Square square = { 5 };
    // CHECK: 25
    outputBuffer[13] = areaOf(square);
    Value value = { 80 };
    // CHECK: 80
    outputBuffer[14] = getValueOfConstant(value);
}
