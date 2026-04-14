// Implicit pointer conversions between incompatible types are rejected.
// All four directions are tested per reviewer feedback:
// 1. Ptr<struct> -> Ptr<interface>  (memory layout mismatch)
// 2. Ptr<interface> -> Ptr<struct>  (memory layout mismatch)
// 3. Ptr<struct> -> Ptr<struct>    (no struct inheritance in Slang)
// 4. Ptr<interface supertype> -> Ptr<interface subtype> (narrowing)

//TEST:SIMPLE(filecheck=CHECK): -target spirv

interface IBase
{
    int getVal();
}

interface IDerived : IBase
{
    int getExtra();
}

struct Foo : IDerived
{
    int val;
    int getVal() { return val; }
    int getExtra() { return val + 1; }
}

struct Bar
{
    int val;
}

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    Foo f;
    f.val = 5;
    Foo* pf = &f;

    // 1. Ptr<struct> -> Ptr<interface>: rejected (layout mismatch)
    // CHECK: error[E30019]
    // CHECK: not bit equivalent
    IBase* ip = pf;

    // 2. Ptr<interface> -> Ptr<struct>: rejected
    // CHECK: error[E30019]
    Foo* pf2 = ip;

    // 3. Ptr<struct> -> Ptr<another struct>: rejected (no struct inheritance)
    Bar b;
    // CHECK: error[E30019]
    Foo* pf3 = &b;

    // 4. Ptr<interface supertype> -> Ptr<interface subtype>: rejected (narrowing)
    IBase ib;
    IBase* ibp = &ib;
    // CHECK: error[E30019]
    IDerived* dp = ibp;

    outputBuffer[0] = 0;
}
