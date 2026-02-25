// Upcasting an existential (interface-typed) value from a derived
// interface to a base interface currently triggers an ICE.
// Disabled until the fix lands. See https://github.com/shader-slang/slang/issues/10156

//DISABLE_TEST:SIMPLE(filecheck=CHK1):-target hlsl -stage compute -entry scenario1
//DISABLE_TEST:SIMPLE(filecheck=CHK2):-target hlsl -stage compute -entry scenario2

// Non-generic interface hierarchy
interface IBase
{
    float getValue();
}

interface IDerived : IBase
{
    float transform(float input);
}

struct ImplA : IDerived
{
    float v;
    float getValue() { return v; }
    float transform(float input) { return input + v; }
}

struct ImplB : IDerived
{
    float v;
    float getValue() { return v * 2.0; }
    float transform(float input) { return input * v; }
}

// Generic interface hierarchy
interface IGenBase<T : IArithmetic>
{
    T getValue();
}

interface IGenDerived<T : IArithmetic> : IGenBase<T>
{
    T transform(T input);
}

struct GenImplA : IGenDerived<float>
{
    float v;
    float getValue() { return v; }
    float transform(float input) { return input + v; }
}

struct GenImplB : IGenDerived<float>
{
    float v;
    float getValue() { return v * 2.0; }
    float transform(float input) { return input * v; }
}

RWStructuredBuffer<float> outputBuffer;

// Scenario 1: Non-generic upcast
[numthreads(1, 1, 1)]
void scenario1()
{
    IDerived derived;
    if (outputBuffer[0] == 0.0)
        derived = ImplA(10.0);
    else
        derived = ImplB(10.0);

    IBase base = derived;
    outputBuffer[0] = base.getValue();
}

// Scenario 2: Generic upcast
[numthreads(1, 1, 1)]
void scenario2()
{
    IGenDerived<float> derived;
    if (outputBuffer[0] == 0.0)
        derived = GenImplA(10.0);
    else
        derived = GenImplB(10.0);

    IGenBase<float> base = derived;
    outputBuffer[0] = base.getValue();
}
