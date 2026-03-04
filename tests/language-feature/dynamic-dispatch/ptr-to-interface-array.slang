// An array of concrete-type pointers used through an implementation that
// contains a pointer field. Each element dispatches to the same concrete type
// but with different pointed-to data. Tests that pointer arrays survive
// AnyValue packing as raw pointer bits.
//
// Note: An array of IFoo* where each element points to a different concrete
// type is not feasible from local code because taking &local of an
// interface-typed variable is not supported. This test uses concrete pointers
// within an implementation instead.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type -emit-spirv-directly
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

interface ICompute
{
    int compute();
}

// Implementation with an array of int pointers.
struct ArrayPtrImpl : ICompute
{
    int* values[2];

    int compute()
    {
        return *values[0] + *values[1];
    }
}

struct DirectImpl : ICompute
{
    int a;
    int b;

    int compute()
    {
        return a * b;
    }
}

ICompute makeArrayPtr(int* p0, int* p1)
{
    ArrayPtrImpl impl;
    impl.values[0] = p0;
    impl.values[1] = p1;
    return impl;
}

ICompute makeDirect(int a, int b)
{
    DirectImpl impl;
    impl.a = a;
    impl.b = b;
    return impl;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    int x = 20;
    int y = 22;

    // ArrayPtrImpl: *values[0] + *values[1] = 20 + 22 = 42
    var comp = makeArrayPtr(&x, &y);
    // CHECK: 42
    outputBuffer[0] = comp.compute();

    // DirectImpl: a * b = 7 * 8 = 56
    var comp2 = makeDirect(7, 8);
    // CHECK: 56
    outputBuffer[1] = comp2.compute();

    // Modify pointed-to values.
    x = 100;
    y = -50;
    var comp3 = makeArrayPtr(&x, &y);
    // CHECK: 50
    outputBuffer[2] = comp3.compute();
}
