//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
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

int defaultedTypeWithArg<T = int>(T x)
{
    if (T is float)
        return 0;
    else if (T is int)
        return 1;
    return 2;
}

int dependentDefault<T, U = T>()
{
    return sizeof(U);
}

int inferredDependentDefaultChain<T, U = T, V = U>(T a)
{
    return sizeof(V);
}

struct MemberFuncs
{
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

    int defaultedTypeWithArg<T = int>(T x)
    {
        if (T is float)
            return 0;
        else if (T is int)
            return 1;
        return 2;
    }

    int dependentDefault<T, U = T>()
    {
        return sizeof(U);
    }

    int inferredDependentDefaultChain<T, U = T, V = U>(T a)
    {
        return sizeof(V);
    }
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
    // CHECK: 1
    outputBuffer[10] = defaultedTypeWithArg(42);
    // CHECK: 0
    outputBuffer[11] = defaultedTypeWithArg(42.0);
    // CHECK: 16
    outputBuffer[12] = dependentDefault<float4>();  // U defaults to float4
    // CHECK: 16
    outputBuffer[13] = inferredDependentDefaultChain(float4(0));

    MemberFuncs m;
    // CHECK: 4
    outputBuffer[14] = m.defaultedTypeSingle();
    // CHECK: 8
    outputBuffer[15] = m.defaultedTypeSingle<float2>();
    // CHECK: 64
    outputBuffer[16] = m.defaultedTypeTwo<float4>();
    // CHECK: 256
    outputBuffer[17] = m.defaultedTypeTwo<float4, float4>();
    // CHECK: 10
    outputBuffer[18] = m.defaultedValueSingle();
    // CHECK: 8
    outputBuffer[19] = m.defaultedValueSingle<8>();
    // CHECK: 160
    outputBuffer[20] = m.defaultedValueTwo<16>();
    // CHECK: 256
    outputBuffer[21] = m.defaultedValueTwo<16, 16>();
    // CHECK: 12
    outputBuffer[22] = m.defaultedConstrainedType();
    // CHECK: 16
    outputBuffer[23] = m.defaultedConstrainedType<Foo2>();
    // CHECK: 1
    outputBuffer[24] = m.defaultedTypeWithArg(42);
    // CHECK: 0
    outputBuffer[25] = m.defaultedTypeWithArg(42.0);
    // CHECK: 16
    outputBuffer[26] = m.dependentDefault<float4>();  // U defaults to float4
    // CHECK: 16
    outputBuffer[27] = m.inferredDependentDefaultChain(float4(0));
}
