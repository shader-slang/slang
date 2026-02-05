//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type

// Test for issue #9836: Interface implementation containing pointer to interface
//
// An implementation of IFoo SHOULD be allowed to contain IFoo* (pointer to interface)
// since pointers have fixed size regardless of pointee.

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

interface IFoo
{
    float foo();
}

// Should WORK - pointer has fixed size
struct GoodImpl : IFoo
{
    IFoo* next; // OK: pointer to interface
    float value;

    float foo() { return value; }
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    GoodImpl a;
    a.value = 1.0;
    a.next = nullptr;

    GoodImpl b;
    b.value = 2.0;
    b.next = nullptr;

    // This is the key test: assigning Ptr<GoodImpl> to IFoo* should work
    // because GoodImpl implements IFoo and pointers have fixed size.
    a.next = &b;

    // Test dynamic dispatch through interface variable
    IFoo iface = a;
    outputBuffer[0] = iface.foo(); // CHECK: 1.0

    // Verify the pointer was assigned correctly by checking it's not null
    outputBuffer[1] = (a.next != nullptr) ? 1.0 : 0.0; // CHECK: 1.0
}
