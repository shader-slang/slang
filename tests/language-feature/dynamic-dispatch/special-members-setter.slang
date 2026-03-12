//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type

//
// Property setters and subscript setters through dynamic dispatch.
//
// Verifies that setting properties and subscripts on an interface-typed variable
// (dynamic dispatch) correctly repacks the modified value back into AnyValue storage.
//

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

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


[numthreads(1, 1, 1)]
void computeMain(int id : SV_DispatchThreadID)
{
    IFullFeatured obj = makeObj(id);

    // Property setter through dynamic dispatch
    obj.value = 5.0;

    // Subscript setter through dynamic dispatch
    obj[0] = 3.0;

    // CHECK: 5.0
    outputBuffer[0] = obj.value;
    // CHECK: 3.0
    outputBuffer[1] = obj[0];
}
