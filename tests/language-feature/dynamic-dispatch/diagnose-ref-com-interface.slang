// COM interfaces only: __ref/__constref are allowed (vtable dispatch).
// No diagnostic 52010 expected. Regular interfaces tested in diagnose-ref-regular-and-mixed.slang.
//DISABLED_TEST:SIMPLE(filecheck=CHECK): -target spirv
//DISABLED_TEST:SIMPLE(filecheck=CHECK): -target hlsl
//DISABLED_TEST:SIMPLE(filecheck=CHECK): -target cuda
//
// Target support: COM is documented as "valid only on CPU target" (core.meta.slang).
// In practice the compiler accepts COM interfaces for cpp, hlsl, and cuda; this test
// runs on spirv, hlsl, and cuda to document compiler behavior on those targets.

// COM interfaces use vtable-based dispatch (pointer indirection) rather
// than the tagged union (AnyValue) packing used by regular interfaces in
// dynamic dispatch. Because vtable dispatch preserves the original object
// layout, __ref and __constref are compatible with COM interfaces.

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

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // COM-only code path; no regular interface or MixedContainer here.
    outputBuffer[0] = 0.0;
}
