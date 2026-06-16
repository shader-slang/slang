//TEST:SIMPLE(filecheck=CHECK): -target spirv -capability abort -skip-spirv-validation

// End-to-end coverage for abort() inside code using an interface-typed
// (existential) value: the `t.val()` condition uses dynamic interface dispatch,
// and passing `t` itself to abort makes legalizeAbort forward the pair-flavor
// ordinary half. The resulting message payload must carry the scalar uint
// value, not the existential metadata side.

interface IThing
{
    uint val();
}

extension uint : IThing
{
    uint val() { return this; }
}

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    IThing t = tid.x;
    if (t.val() > 100)
    {
        abort("val=%u", t);
    }
}

// CHECK: OpCapability AbortKHR
// CHECK: %AbortMessage = OpTypeStruct %_arr_uint_int_2 %uint
// CHECK: OpAbortKHR %AbortMessage
