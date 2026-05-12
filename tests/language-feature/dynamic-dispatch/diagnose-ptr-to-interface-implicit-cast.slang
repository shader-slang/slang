// Implicit pointer conversions between incompatible types are rejected.
// All four directions are tested per reviewer feedback:
// 1. Ptr<struct> -> Ptr<interface>  (memory layout mismatch)
// 2. Ptr<interface> -> Ptr<struct>  (memory layout mismatch)
// 3. Ptr<struct> -> Ptr<struct>    (no struct inheritance in Slang)
// 4. Ptr<interface supertype> -> Ptr<interface subtype> (narrowing)

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_2026,non-exhaustive): -std 2026 -target spirv
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_2025,non-exhaustive): -std 2025 -target spirv

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
    // CHECK_2026: error[E31207]
    // CHECK_2025: warning[E31200]
    Foo* pf = &f;

    // 1. Ptr<struct> -> Ptr<interface>: rejected (layout mismatch)
    // CHECK_2026: error[E30019]
    // CHECK_2026: not bit equivalent
    // CHECK_2025: error[E30019]
    // CHECK_2025: not bit equivalent
    IBase* ip = pf;

    // 2. Ptr<interface> -> Ptr<struct>: rejected
    // CHECK_2026: error[E30019]
    // CHECK_2026-NOT: not bit equivalent
    // CHECK_2025: error[E30019]
    // CHECK_2025-NOT: not bit equivalent
    Foo* pf2 = ip;

    // 3. Ptr<struct> -> Ptr<another struct>: rejected (no struct inheritance)
    Bar b;
    // CHECK_2026: error[E31207]
    // CHECK_2026: error[E30019]
    // CHECK_2026-NOT: not bit equivalent
    // CHECK_2025: warning[E31200]
    // CHECK_2025: error[E30019]
    // CHECK_2025-NOT: not bit equivalent
    Foo* pf3 = &b;

    // 4. Ptr<interface supertype> -> Ptr<interface subtype>: rejected (narrowing)
    IBase ib;
    // CHECK_2026: error[E31207]
    // CHECK_2025: warning[E31200]
    IBase* ibp = &ib;
    // CHECK_2026: error[E30019]
    // CHECK_2026-NOT: not bit equivalent
    // CHECK_2025: error[E30019]
    // CHECK_2025-NOT: not bit equivalent
    IDerived* dp = ibp;

    outputBuffer[0] = 0;
}
