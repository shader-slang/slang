// this-in-extension.slang

// Test that an `This` type works correctly when there is an extension.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -shaderobj

interface IFoo
{
    static const This identityZero;   
    static const This identityFive;
}

__generic<T:IFoo>
extension T
{
    This getIdentityZero()
    {
        return identityZero;
    }
    This getIdentityFive()
    {
        return identityFive;
    }
}

struct FooImpl : IFoo
{
    int v = 1;
    static const This identityZero = This();   
    static const This identityFive = This(5);
}


//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(int3 dispatchThreadID : SV_DispatchThreadID)
{
    FooImpl foo;
    var i0 = foo.getIdentityZero();
    var i5 = foo.getIdentityFive();
    // CHECK: 1
    outputBuffer[0] = i0.v;
    // CHECK: 5
    outputBuffer[1] = i5.v;
}
