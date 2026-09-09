// Upcasting an existential (interface-typed) value from a derived
// interface to a base interface should produce correct dispatch code.

//TEST:SIMPLE(filecheck=CHK1):-target hlsl -stage compute -entry scenario1

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

RWStructuredBuffer<float> outputBuffer;

// Scenario 1: Non-generic upcast — verify the compiler lowers `base.getValue()`
// to a dispatch through the witness-table chain walked at the upcast (IDerived's
// `$inheritance` entry → IBase's `getValue`), not a single direct call that would
// indicate the upcast was folded away to one concrete type. The dispatch itself
// is force-inlined into `scenario1` (see slang-ir-lower-dynamic-dispatch-insts.cpp),
// so what's checked is the surviving `switch` with calls into both concrete
// implementations, not a separately named dispatch function. The two calls
// below are matched order-independently (not sequentially): the switch-case
// order between ImplA and ImplB is not guaranteed across compilers/platforms
// (confirmed -- MSVC and GCC emit them in opposite orders).
// CHK1: void scenario1
// CHK1: switch
// CHK1-DAG: ImplB_getValue
// CHK1-DAG: ImplA_getValue
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
