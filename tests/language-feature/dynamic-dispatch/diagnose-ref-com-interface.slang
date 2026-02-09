//TEST:SIMPLE(filecheck=CHECK): -target spirv
//TEST:SIMPLE(filecheck=CHECK): -target hlsl
//TEST:SIMPLE(filecheck=CHECK): -target cuda

// COM interfaces use vtable-based dispatch (pointer indirection) rather
// than the tagged union (AnyValue) packing used by regular interfaces in
// dynamic dispatch. Because vtable dispatch preserves the original object
// layout, __ref and __constref are compatible with COM interfaces.
//
// This test verifies COM interfaces are accepted while regular interfaces
// are still rejected.

[com_interface]
[uuid("00000000-0000-0000-0000-000000000001")]
interface ICOMExample
{
    float getValue();
    void setValue(float v);
}

// CHECK-NOT: error 52010{{.*}}ICOMExample
// CHECK-NOT: error 52010{{.*}}COMContainer

// Vtable dispatch: __ref to a COM interface is a direct pointer to
// the object, so reference semantics work correctly.
float processCOMRef(__ref ICOMExample obj)
{
    return obj.getValue();
}

float processCOMConstref(__constref ICOMExample obj)
{
    return obj.getValue();
}

// Mutation through __ref is also valid for COM interfaces since the
// reference points to the actual object, not a packed representation.
void modifyCOMRef(__ref ICOMExample obj)
{
    obj.setValue(42.0);
}

// A struct whose only interface member is COM should also be allowed,
// since no AnyValue packing is involved.
struct COMContainer
{
    ICOMExample comObj;
}

float processCOMContainerRef(__ref COMContainer c)
{
    return c.comObj.getValue();
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
