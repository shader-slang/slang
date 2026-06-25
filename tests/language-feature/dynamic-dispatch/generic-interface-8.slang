// One-argument `createDynamicObject<T>(typeId)` now works for all three
// generic-interface shapes that #10063 documented as failing:
//   1. Concrete specialization: createDynamicObject<ICalculator<float>>
//   2. Open type parameter:     createDynamicObject<ICalculator<T>>
//   3. Interface type argument: createDynamicObject<ICalculator<IFoo>>

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type

interface IFoo
{
    float eval();
}

struct FooA : IFoo
{
    float eval() { return 7.0; }
}

interface ICalculator<T>
{
    T compute(T a, T b);
}

struct Adder : ICalculator<float>
{
    float compute(float a, float b) { return a + b; }
}

struct Multiplier : ICalculator<float>
{
    float compute(float a, float b) { return a * b; }
}

struct FooCalculator : ICalculator<IFoo>
{
    IFoo compute(IFoo a, IFoo b) { return a; }
}

//TEST_INPUT: type_conformance Adder:ICalculator<float> = 0
//TEST_INPUT: type_conformance Multiplier:ICalculator<float> = 1
//TEST_INPUT: type_conformance FooA:IFoo = 2
//TEST_INPUT: type_conformance FooCalculator:ICalculator<IFoo> = 3

// Case 1: Concrete specialization — dispatch through ICalculator<float>.
ICalculator<float> createCalcFloat(uint id)
{
    return createDynamicObject<ICalculator<float>>(id);
}

// Case 2: Open type parameter — the generic is re-specialized at each caller.
ICalculator<T> createCalcGeneric<T>(uint id)
{
    return createDynamicObject<ICalculator<T>>(id);
}

// Case 3: Interface type as type argument.
ICalculator<IFoo> createCalcInterface(uint id)
{
    return createDynamicObject<ICalculator<IFoo>>(id);
}

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    let calcFloat = createCalcFloat(0);
    outputBuffer[0] = calcFloat.compute(3.0, 4.0);
    // Adder(3, 4) = 7
    // CHECK: 7.0

    let calcGeneric = createCalcGeneric<float>(1);
    outputBuffer[1] = calcGeneric.compute(3.0, 4.0);
    // Multiplier(3, 4) = 12
    // CHECK-NEXT: 12.0

    let calcInterface = createCalcInterface(3);
    let foo = createDynamicObject<IFoo>(2);
    // FooCalculator returns its first operand; foo.eval() = 7.
    outputBuffer[2] = calcInterface.compute(foo, foo).eval();
    // CHECK-NEXT: 7.0
}
