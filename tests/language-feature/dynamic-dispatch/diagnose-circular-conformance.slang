// Circular dependencies in interface conformance are invalid for dynamic dispatch.
//
// Dynamic dispatch uses a tagged union (AnyValue) to store conforming types at
// runtime. If a conforming type contains a field of the interface type it
// implements, this creates infinite recursion: packing the struct requires
// knowing its size, but the interface field's size depends on all conforming
// types, including this one.
//
// The compiler should detect this circularity and report an error. Currently
// the error manifests as "cannot be packed" (41014) or "does not fit in size"
// (41011), but ideally would be a specific circular dependency diagnostic.

//TEST:SIMPLE(filecheck=CHECK): -target spirv

// Direct circular dependency: struct contains the interface it implements
[anyValueSize(32)]
interface IValue
{
    float get();
}

// CHECK: error {{[0-9]+}}
struct BadImpl : IValue
{
    IValue child;  // Circular: BadImpl contains IValue, BadImpl conforms to IValue
    float get() { return 0; }
}

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    IValue v = createDynamicObject<IValue>(0, 5);
    outputBuffer[0] = v.get();
}
