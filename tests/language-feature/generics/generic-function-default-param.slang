//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

interface IFoo
{
}

struct Foo1 : IFoo
{
    int a;
    int b;
    int c;
}

struct Foo2 : IFoo
{
    int a;
    int b;
    int c;
    int d;
}

int defaultedConstrainedType<T : IFoo = Foo1>()
{
    return sizeof(T);
}

int defaultedTypeSingle<U = int>()
{
    return sizeof(U);
}

int defaultedTypeTwo<T, U = int>()
{
    return sizeof(T) * sizeof(U);
}

int defaultedValueSingle<int a = 10>()
{
    return a;
}

int defaultedValueTwo<int a, int b = 10>()
{
    return a * b;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    // CHECK: 4
    outputBuffer[0] = defaultedTypeSingle();
    // CHECK: 8
    outputBuffer[1] = defaultedTypeSingle<float2>();
    // CHECK: 64
    outputBuffer[2] = defaultedTypeTwo<float4>();
    // CHECK: 256
    outputBuffer[3] = defaultedTypeTwo<float4, float4>();
    // CHECK: 10
    outputBuffer[4] = defaultedValueSingle();
    // CHECK: 8
    outputBuffer[5] = defaultedValueSingle<8>();
    // CHECK: 160
    outputBuffer[6] = defaultedValueTwo<16>();
    // CHECK: 256
    outputBuffer[7] = defaultedValueTwo<16, 16>();
    // CHECK: 12
    outputBuffer[8] = defaultedConstrainedType();
    // CHECK: 16
    outputBuffer[9] = defaultedConstrainedType<Foo2>();
}
