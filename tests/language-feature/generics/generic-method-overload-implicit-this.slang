// Regression test for https://github.com/shader-slang/slang/issues/9678
//
// When a generic method overload calls another generic method overload on the
// same struct (using implicit `this`), the compiler must include `this` in the
// IR call arguments. Previously this triggered an assert in constexpr
// propagation because the callee function type expected the `this` parameter
// but the call omitted it.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -shaderobj -output-using-type

struct Foo
{
    int x;

    int load<T>(int handle)
    {
        return load<T>(handle, 0);
    }

    int load<T>(int handle, int offset)
    {
        return x + handle + offset;
    }
};

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    Foo foo;
    foo.x = 100;
    // CHECK: 105
    outputBuffer[0] = foo.load<float>(5);
}
