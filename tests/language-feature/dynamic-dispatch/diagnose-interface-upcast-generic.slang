// Generic interface upcast triggers a separate ICE (not fixed by #10850).
// Split from diagnose-interface-upcast.slang because the generic case
// crashes the compiler during global passes, blocking non-generic tests.

//DISABLE_TEST:SIMPLE(filecheck=CHK2):-target hlsl -stage compute -entry scenario2

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

// Scenario 2: Generic upcast
// CHK2: void scenario2
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
