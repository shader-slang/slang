//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -shaderobj
//TEST(compute,vulkan):COMPARE_COMPUTE_EX:-vk -slang -compute -shaderobj

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

// TODO(tfoley): What this test is testing seems to be outside
// the scope of what we ever intend to support in user code.
// Returning an `Ref<T>` is supposed to be a core-module-only
// thing, and even then is something that we would like to do less
// of over time.
//
// The only purpose of this test *seems* to be ensuring that this
// particular function (and the `groupshared` declaration inside
// it) "works," but the function itself is not something that we
// intend to be supported in Slang.
// 
[ForceInline]
Ref<uint> table<let n: int>(int index)
{
    static groupshared uint array[n];
    return array[index];
}

struct S<let n : int>
{
    static const int M = n * 2;
    int doSomething()
    {
        table<M>(0) = M;
        return int(table<M>(0));
    }
}

[numthreads(1, 1, 1)]
void computeMain(int3 dispatchThreadID: SV_DispatchThreadID)
{
    S<2> s;
    outputBuffer[0] = s.doSomething();
}
