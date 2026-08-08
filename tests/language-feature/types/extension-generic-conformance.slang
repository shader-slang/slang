// extension-generic-conformance.slang

// Test adding interface conformance to a generic type via extension.
// The extension provides a default implementation for all instantiations
// that satisfy the constraint.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -shaderobj -output-using-type

interface IStringLength
{
    int length();
}

struct Wrapper<T : IArithmetic>
{
    T value;
}

extension<T : IArithmetic> Wrapper<T> : IStringLength
{
    int length()
    {
        return 1;
    }
}

int getLen<U : IStringLength>(U obj)
{
    return obj.length();
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    Wrapper<int> wi;
    wi.value = 42;

    Wrapper<float> wf;
    wf.value = 3.14;

    // CHECK: 1
    outputBuffer[0] = getLen(wi);
    // CHECK: 1
    outputBuffer[1] = getLen(wf);
    // CHECK: 42
    outputBuffer[2] = wi.value;
    // CHECK: 3
    outputBuffer[3] = int(wf.value);
}
