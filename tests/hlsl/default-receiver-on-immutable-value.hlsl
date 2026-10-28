//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type

//TEST:SIMPLE(filecheck=DXIL): -target dxil-asm -entry computeMain -profile cs_6_0

// An HLSL method without a receiver specifier takes `this` as `inout`, so that it can modify a
// mutable receiver. In HLSL code, a call can also pass a copy of an immutable receiver argument to
// an `inout` `this` parameter: a constant buffer member, a `StructuredBuffer` element, a constant,
// a temporary or an existential value. DXC accepts such calls for methods that do not write
// `this`. The callee's writes to `this` stay in the copy, so they do not reach the immutable
// storage (issue #13964).

// The DXIL run checks that DXC validates the HLSL that Slang emits for these calls, in particular
// that a copied receiver does not store to a constant buffer or `StructuredBuffer`.
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

    [mutating]
    int mutatingAddAndGet(int delta)
    {
        value += delta;
        return value;
    }

    // Inside a trailing-`const` method, `this` is immutable, so the implicit `this.get()` call
    // passes a copy.
    int getTwice() const { return get() + get(); }
};

// A default HLSL method also satisfies a requirement with an `in` receiver, such as those of the
// core module's `IComparable` and of the Slang interface `IShape`. Conformance synthesizes a
// wrapper that calls the method on its immutable `this`, and that call is checked as HLSL code.
struct Key : IComparable
{
    int key;

    bool lessThan(Key other) { return key < other.key; }
    bool lessThanOrEquals(Key other) { return key <= other.key; }
    bool equals(Key other) { return key == other.key; }
};

// An explicit `[mutating]` method has an `inout` receiver too, so it also meets the requirement.
struct MutatingKey : IComparable
{
    int key;

    [mutating]
    bool lessThan(MutatingKey other) { return key < other.key; }
    [mutating]
    bool lessThanOrEquals(MutatingKey other) { return key <= other.key; }
    [mutating]
    bool equals(MutatingKey other) { return key == other.key; }
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

// A generic call through an `inout` requirement also accepts an immutable receiver argument.
interface IValue
{
    int getValue();
    int addAndGetValue(int delta);
};

struct Value : IValue
{
    int value;

    int getValue() { return value; }

    int addAndGetValue(int delta)
    {
        value += delta;
        return value;
    }
};

int getValueOfConstant<T : IValue>(T value)
{
    const T constant = value;
    return constant.getValue();
}

int addAndGetValueOfConstant<T : IValue>(T value, int delta)
{
    const T constant = value;
    return constant.addAndGetValue(delta);
}

IValue makeValue(int value)
{
    Value result = { value };
    return result;
}

// On an immutable receiver argument, an overload whose `this` needs no copy is preferred.
struct Overloaded
{
    int value;

    int choose() { return 1; }
};

extension Overloaded
{
    [nonmutating]
    int choose() { return 2; }
};

static const Overloaded kOverloaded = { 0 };

// A call through an `inout` requirement whose witness is a `[__ref]` method runs the synthesized
// `inout` wrapper on the copy, and the wrapper passes its own `this` to the method.
interface IBump
{
    int bump();
};

struct RefBump : IBump
{
    int value;

    [__ref]
    int bump() { return ++value; }
};

static const RefBump kRefBump = { 5 };

Helper makeHelper(int value)
{
    Helper helper;
    helper.value = value;
    return helper;
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

//TEST_INPUT: set gSlangCounters = ubuffer(data=[100], stride=4)
StructuredBuffer<SlangCounter> gSlangCounters;

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4)
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
    outputBuffer[6] = gHelpers[0].get();

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

    // Writes through other immutable receiver arguments also stay in the copy.
    // CHECK: 51
    outputBuffer[15] = kHelper.addAndGet(1);
    // CHECK: 50
    outputBuffer[16] = kHelper.get();
    // CHECK: 71
    outputBuffer[17] = localHelper.addAndGet(1);
    // CHECK: 70
    outputBuffer[18] = localHelper.get();
    // CHECK: 61
    outputBuffer[19] = makeHelper(60).addAndGet(1);
    // CHECK: 81
    outputBuffer[20] = addAndGetValueOfConstant(value, 1);

    // An existential value is an immutable receiver argument too.
    const IValue constantValue = makeValue(90);
    // CHECK: 90
    outputBuffer[21] = constantValue.getValue();
    // CHECK: 91
    outputBuffer[22] = constantValue.addAndGetValue(1);
    // CHECK: 90
    outputBuffer[23] = constantValue.getValue();
    // CHECK: 80
    outputBuffer[24] = value.getValue();

    // CHECK: 2
    outputBuffer[25] = kOverloaded.choose();

    // Explicit `[mutating]` methods, written in HLSL or in Slang, also receive a copy when this HLSL
    // code calls them on an immutable receiver argument.
    // CHECK: 12
    outputBuffer[26] = gHelper.mutatingAddAndGet(2);
    // CHECK: 10
    outputBuffer[27] = gHelper.get();
    // CHECK: 101
    outputBuffer[28] = gSlangCounters[0].addAndGet(1);
    // CHECK: 100
    outputBuffer[29] = gSlangCounters[0].value;
    // CHECK: 20
    outputBuffer[30] = gHelper.getTwice();
    MutatingKey c = { 7 };
    MutatingKey d = { 6 };
    // CHECK: 6
    outputBuffer[31] = minOf(c, d).key;

    // CHECK: 6
    outputBuffer[32] = kRefBump.bump();
    // CHECK: 5
    outputBuffer[33] = kRefBump.value;

    // On a mutable receiver argument, neither overload needs a copy, and the one declared in the
    // type is chosen.
    Overloaded mutableOverloaded = { 0 };
    // CHECK: 1
    outputBuffer[34] = mutableOverloaded.choose();
}
