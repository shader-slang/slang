// The issue's literal repro shape: an inherited field written and read
// through a ref-only property. This is compile-only coverage: lowering must
// not abort with E99997 "unexpected member flavor" (the pre-fix crash), and
// the write exercises tryGetAddress's ref-accessor path, which resolves the
// deferred base value to a pointer. Executing this shape is still blocked
// downstream by #9636 (ref accessors emit invalid code: HLSL references an
// undefined Ptr<> type, and CPU and CUDA returns are pointer-mismatched), so
// execution coverage lands with that issue instead of here.
//TEST:SIMPLE(filecheck=CHECK): -target spirv -entry main -stage compute

//CHECK: OpCapability Shader

#pragma warning(disable:30816)

struct Base
{
    float value;
}

struct Payload : Base {}

struct Holder
{
    Payload stored = {};

    property Payload payload
    {
        ref { return stored; }
    }
}

RWStructuredBuffer<float> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    Holder h = {};
    h.payload.value = 1.0;
    output[0] = h.stored.value;
}
