//TEST:SIMPLE(filecheck=CHECK): -target spirv -capability abort -skip-spirv-validation

// Regression test for the `pair`-flavor branch of legalizeAbort in
// slang-ir-legalize-types.cpp: an interface-typed (existential) value splits
// into resource + ordinary halves during type legalization, and the abort
// must forward only the ordinary half. Primitive-scalar arguments only ever
// exercise the `simple` flavor.

interface IThing
{
    uint val();
}

struct Concrete : IThing
{
    uint v;
    uint val() { return v; }
}

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    IThing t = Concrete(tid.x);
    if (t.val() > 100)
    {
        abort("val=%u", t.val());
    }
}

// CHECK: OpCapability AbortKHR
// CHECK: OpAbortKHR
