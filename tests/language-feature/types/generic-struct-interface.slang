// generic-struct-interface.slang

// Test a generic struct implementing a generic interface.
// Exercises type parameter threading through conformance checking.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -shaderobj -output-using-type

interface ITransform<T>
{
    T apply(T value);
}

struct ScaleBy<T : IArithmetic> : ITransform<T>
{
    T factor;

    __init(T f)
    {
        factor = f;
    }

    T apply(T value)
    {
        return value * factor;
    }
}

struct Offset<T : IArithmetic> : ITransform<T>
{
    T offset;

    __init(T o)
    {
        offset = o;
    }

    T apply(T value)
    {
        return value + offset;
    }
}

T applyTransform<T, U : ITransform<T>>(U transform, T value)
{
    return transform.apply(value);
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    ScaleBy<int> scaler = ScaleBy<int>(3);
    Offset<int> adder = Offset<int>(10);

    // CHECK: 15
    outputBuffer[0] = applyTransform(scaler, 5);
    // CHECK: 17
    outputBuffer[1] = applyTransform(adder, 7);
    // CHECK: 30
    outputBuffer[2] = scaler.apply(adder.apply(0));
    // CHECK: 19
    outputBuffer[3] = adder.apply(scaler.apply(3));
}
