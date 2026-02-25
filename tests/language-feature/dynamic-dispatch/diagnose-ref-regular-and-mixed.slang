// Tests disabled: diagnostic 52010 not yet implemented (see #9906).
// Enable when __ref/__constref dynamic dispatch diagnostic is added.
//DISABLED_TEST:SIMPLE(filecheck=CHECK): -target spirv
//DISABLED_TEST:SIMPLE(filecheck=CHECK): -target hlsl
//DISABLED_TEST:SIMPLE(filecheck=CHECK): -target cuda

// Regular (non-COM) interfaces and structs containing both COM and regular
// interface members must still be rejected for __ref/__constref in dynamic
// dispatch. This test runs in a separate invocation from the COM-only test
// so that COM (no diagnostics) and regular/mixed (diagnostics) are tested
// independently.

[com_interface]
[uuid("00000000-0000-0000-0000-000000000001")]
interface ICOMExample
{
    float getValue();
    void setValue(float v);
}

// Regular (non-COM) interfaces use AnyValue packing, so __ref is invalid.
[anyValueSize(16)]
interface IRegular
{
    float getValue();
}

struct RegularImpl : IRegular
{
    float v;
    float getValue() { return v; }
}

// CHECK: ([[# @LINE+1]]): error 52010: '__ref' parameter of type 'IRegular' cannot be used in dynamic dispatch context.
float processRegularRef(__ref IRegular obj)
{
    return obj.getValue();
}

// A struct containing both a COM and a regular interface member must
// still be rejected, because the regular member requires AnyValue packing.
struct MixedContainer
{
    ICOMExample comObj;
    IRegular regularObj;
}

// CHECK: ([[# @LINE+1]]): error 52010: '__ref' parameter of type 'MixedContainer' cannot be used in dynamic dispatch context.
float processMixedRef(__ref MixedContainer c)
{
    return c.comObj.getValue() + c.regularObj.getValue();
}

// Verify no spurious errors after the expected diagnostics.
// CHECK-NOT: error 52010

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    IRegular reg = RegularImpl(5.0);
    outputBuffer[0] = processRegularRef(reg);

    MixedContainer mixed;
    mixed.regularObj = reg;
    outputBuffer[1] = processMixedRef(mixed);
}
