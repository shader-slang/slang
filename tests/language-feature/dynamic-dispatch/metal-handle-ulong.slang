// Regression test for https://github.com/shader-slang/slang/issues/11313
//
// Dynamic-dispatch handle types (WitnessTableID / RTTIHandle) are lowered to a
// fixed-layout 64-bit representation in lowerHandleTypes(). On most targets
// that is `uint2`, chosen (PR #9386) so SPIR-V output would not require the
// Int64 capability. Metal cannot cast a vector to a pointer, so a handle that
// is later bit-cast to a `device T*` produced `(device T*)(uint2)`, which the
// Metal compiler rejects. Metal fully supports `ulong`, so on Metal these
// handles are lowered to scalar `ulong` instead, keeping the 8-byte layout but
// making the pointer cast legal. This test pins that Metal `ulong` lowering:
// the lowered existential's handle fields must appear as `ulong`, never the
// `uint2` that the other targets use.

//TEST:SIMPLE(filecheck=MTL): -target metal -stage compute -entry computeMain -conformance "ImplA:IFoo=0" -conformance "ImplB:IFoo=1"

// Before the fix the lowered handle fields were `uint2`; on Metal they must be
// scalar `ulong`. (No other 64-bit scalar appears in this shader.)
// MTL: ulong

[anyValueSize(8)]
interface IFoo
{
    int getValue();
}

struct ImplA : IFoo
{
    int v;
    int getValue() { return v; }
}

struct ImplB : IFoo
{
    int v;
    int getValue() { return v * 2; }
}

// Wrapping an interface-typed field in a ConstantBuffer forces the lowered
// existential layout (RTTI handle + WitnessTableID handle + any-value payload)
// to be emitted in the Metal source, so the handle representation is visible.
struct Params
{
    IFoo foo;
}
ConstantBuffer<Params> gParams;

RWStructuredBuffer<int> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = gParams.foo.getValue();
}
