//TEST:SIMPLE(filecheck=CHECK): -target spirv -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK): -target cpp -entry computeMain -stage compute

//
// Limitation: Property setters and subscript setters through dynamic dispatch
//
// When a property setter or subscript setter is invoked through an interface-typed variable
// (dynamic dispatch), the compiler emits a controlled error (not a crash/segfault):
//   error 99999: unimplemented: Unimplemented type packing
//
// The mutation requires repacking the modified concrete value back into the AnyValue
// storage of the existential interface value, which is not yet implemented.
//
// When this limitation is fixed, this test should be replaced with a positive
// compute test verifying correct runtime behavior.
//

interface IFullFeatured
{
    property float value { get; set; }
    __subscript(int i) -> float { get; set; }
}

struct ImplA : IFullFeatured
{
    int _val;
    float _storage;
    property float value
    {
        get { return float(_val); }
        set { _val = int(newValue); }
    }
    __subscript(int i) -> float
    {
        get { return _storage; }
        set { _storage = newValue; }
    }
}

struct ImplB : IFullFeatured
{
    int _val;
    float _storage;
    float _scale;
    property float value
    {
        get { return float(_val) * _scale; }
        set { _val = int(newValue / _scale); }
    }
    __subscript(int i) -> float
    {
        get { return _storage * _scale; }
        set { _storage = newValue / _scale; }
    }
}

IFullFeatured makeObj(int id)
{
    if (id == 0) { ImplA a; a._val = 10; a._storage = 0.0; return a; }
    else { ImplB b; b._val = 20; b._storage = 0.0; b._scale = 2.0; return b; }
}

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(int id : SV_DispatchThreadID)
{
    IFullFeatured obj = makeObj(id);

    // Property setter through dynamic dispatch — emits error 99999: unimplemented type packing
    // CHECK: error 99999
    obj.value = 5.0;

    // Subscript setter through dynamic dispatch — same limitation
    obj[0] = 3.0;

    outputBuffer[0] = obj.value;
    outputBuffer[1] = obj[0];
}
