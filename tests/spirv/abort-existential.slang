//TEST:SIMPLE(filecheck=CHECK): -target spirv -capability abort -skip-spirv-validation

// End-to-end coverage for abort() inside code using an interface-typed
// (existential) value: the call goes through existential specialization
// before the abort is lowered, and the scalar `t.val()` payload must still
// reach OpAbortKHR. (The `pair`-flavor branch of legalizeAbort is exercised
// separately by tests/diagnostics/abort-pair-arg.slang, which passes a
// resource-containing struct so the argument actually legalizes as a pair.)

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
