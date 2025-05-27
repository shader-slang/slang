//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -dx12
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu

// CHECK: 5
// CHECK-NEXT: 5
// CHECK-NEXT: 5
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

interface IFoo
{
    property uint value { get; }
};

struct Foo : IFoo 
{
    property uint value
    {
        get { return _value; }
    }

    uint _value;
};

void issue1(Foo foo)
{
    int array[10] = {0,1,2,3,4,5,6,7,8,9};
    outputBuffer[0] = array[foo._value]; // ok
    outputBuffer[1] = array[uint(foo.value)]; // ok
    outputBuffer[2] = array[foo.value]; // Used to cause error, 'value' was not resolved before checking that it's an integer
}

void issue2(Foo foo, uint arg)
{
    bool b1 = foo._value < arg; // ok
    bool b2 = uint(foo.value) < arg; // ok
    bool b3 = foo.value < arg; // Used to crash the compiler
    bool b4 = foo.value > arg; // ok
    if (b1) outputBuffer[3] = 1;
    if (b2) outputBuffer[4] = 1;
    if (b3) outputBuffer[5] = 1;
    if (!b4) outputBuffer[6] = 1;
}

[numthreads(1, 1, 1)]
void computeMain(int3 dispatchThreadID: SV_DispatchThreadID)
{
    Foo foo;
    foo._value = 5;
    issue1(foo);
    foo._value = 42;
    issue2(foo, 43);
}
