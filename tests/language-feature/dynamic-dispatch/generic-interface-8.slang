// createDynamicObject does not currently support generic interface types (bug #10063).
// This tests three variants that should all produce clean errors (not crashes):
//   1. Concrete specialization: createDynamicObject<ICalculator<float>>
//   2. Open type parameter:     createDynamicObject<ICalculator<T>>
//   3. Interface type argument:  createDynamicObject<ICalculator<IFoo>>

//TEST:SIMPLE(filecheck=CHECK): -target spirv
//TEST:SIMPLE(filecheck=CHECK): -target hlsl -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=CHECK): -target glsl -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=CHECK): -target metal -stage compute -entry computeMain

RWStructuredBuffer<float> outputBuffer;

interface IFoo
{
    float eval();
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

// Case 1: Concrete type specialization.
// createDynamicObject fails to specialize when given a concrete generic interface
// like ICalculator<float>, producing a misleading "could not specialize generic
// for arguments of type (uint)" error instead of creating the dynamic object.
// See https://github.com/shader-slang/slang/issues/10063
ICalculator<float> createCalcFloat(uint id)
{
    // CHECK: ([[# @LINE+1]]): error 39999
    return createDynamicObject<ICalculator<float>>(id);
}

// Case 2: Open type parameter.
// Same issue as Case 1 but with an unresolved type parameter — the compiler
// cannot specialize createDynamicObject when the interface type argument is
// itself generic.
// See https://github.com/shader-slang/slang/issues/10063
ICalculator<T> createCalcGeneric<T>(uint id)
{
    // CHECK: ([[# @LINE+1]]): error 39999
    return createDynamicObject<ICalculator<T>>(id);
}

// Case 3: Interface type as type argument.
// Using another interface (IFoo) as the type argument to a generic interface
// hits the same createDynamicObject specialization failure.
// See https://github.com/shader-slang/slang/issues/10063
ICalculator<IFoo> createCalcInterface(uint id)
{
    // CHECK: ([[# @LINE+1]]): error 39999
    return createDynamicObject<ICalculator<IFoo>>(id);
}

[numthreads(1, 1, 1)]
void computeMain()
{
    let calcFloat = createCalcFloat(0);
    outputBuffer[0] = calcFloat.compute(3.0, 4.0);

    let calcGeneric = createCalcGeneric<float>(1);
    outputBuffer[1] = calcGeneric.compute(3.0, 4.0);

    let calcInterface = createCalcInterface(2);
}
