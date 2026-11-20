//TEST:COMPARE_COMPUTE(targets=all, filecheck-buffer=CHECK):-output-using-type

interface IValueGeneric<let D : int> {}
struct ValGenericImpl<let D : int> : IValueGeneric<D> {}

struct NestedValueGeneric<let D : int, S : IValueGeneric<D>>
{
    int x;
}

void acceptor<let D : int, S : IValueGeneric<D>>(NestedValueGeneric<D, S> x)
{
    outputBuffer[0] = D + x.x;
}

extension<let D : int, S : IValueGeneric<D>> NestedValueGeneric<D, S>
{
    void foo()
    {
        acceptor(this);
    }
}
void test2(NestedValueGeneric<2, ValGenericImpl<2>> x)
{
    // 'foo' should be a member of 'NestedValueGeneric<2, ValGenericImpl<2>>' through
    // the extension above.
    x.foo();
}

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(int3 dispatchThreadID: SV_DispatchThreadID)
{
    NestedValueGeneric<2, ValGenericImpl<2>> x;
    x.x = 1;
    test2(x);
    // CHECK: 3
}