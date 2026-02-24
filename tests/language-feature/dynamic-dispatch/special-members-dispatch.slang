//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST:SIMPLE(filecheck=REPORT):-target hlsl -stage compute -entry computeMain -report-dynamic-dispatch-sites

// Test for GitHub issue #9887: Dynamic Dispatch of Special Members
// Verifies that property getters, subscript getters, function call operators (operator()),
// and constructors (__init) all work correctly through dynamic dispatch.
//
// Note: Property setters and subscript setters through dynamic dispatch are a known
// limitation (unimplemented type packing). See special-members-setter-limitation.slang.

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

interface IFullFeatured
{
    __init(int value);

    property float value { get; set; }

    __subscript(int i) -> float { get; set; }

    float operator()(float x, float y);

    int getValue();
}

// ImplA: identity implementation
struct ImplA : IFullFeatured
{
    int _val;
    float _storage;

    // __init initializes both fields from value
    __init(int value) { _val = value; _storage = float(value); }

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

    float operator()(float x, float y) { return x + y; }

    int getValue() { return _val; }
}

// ImplB: scaled implementation — multiplies/divides by _scale=2.0
struct ImplB : IFullFeatured
{
    int _val;
    float _storage;
    float _scale;

    // __init doubles _val and initializes _storage and _scale
    __init(int value) { _val = value * 2; _storage = float(value); _scale = 2.0; }

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

    float operator()(float x, float y) { return x * y; }

    int getValue() { return _val; }
}

IFullFeatured makeObj(int id)
{
    if (id == 0) return ImplA(10);
    else return ImplB(10);
}

[numthreads(1, 1, 1)]
void computeMain(int id : SV_DispatchThreadID)
{
    // --- ImplA (id=0): ImplA(10) -> _val=10, _storage=10.0 ---
    IFullFeatured objA = makeObj(id);

    // Property getter via dynamic dispatch
    outputBuffer[0] = objA.value;               // ImplA: float(10) = 10.0
    // REPORT-DAG: ([[# @LINE-1]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
    // CHECK: 10.0

    // Subscript getter via dynamic dispatch
    outputBuffer[1] = objA[0];                  // ImplA: _storage = 10.0
    // REPORT-DAG: ([[# @LINE-1]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
    // CHECK: 10.0

    // Function call operator via dynamic dispatch
    outputBuffer[2] = objA(2.0, 3.0);           // ImplA: 2.0 + 3.0 = 5.0
    // REPORT-DAG: ([[# @LINE-1]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
    // CHECK: 5.0

    // Regular method to verify __init ran correctly
    outputBuffer[3] = float(objA.getValue());   // ImplA: _val = 10
    // REPORT-DAG: ([[# @LINE-1]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
    // CHECK: 10.0

    // --- ImplB (id+1=1): ImplB(10) -> _val=20, _storage=10.0, _scale=2.0 ---
    IFullFeatured objB = makeObj(id + 1);

    // Property getter via dynamic dispatch
    outputBuffer[4] = objB.value;               // ImplB: float(20)*2.0 = 40.0
    // REPORT-DAG: ([[# @LINE-1]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
    // CHECK: 40.0

    // Subscript getter via dynamic dispatch
    outputBuffer[5] = objB[0];                  // ImplB: 10.0*2.0 = 20.0
    // REPORT-DAG: ([[# @LINE-1]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
    // CHECK: 20.0

    // Function call operator via dynamic dispatch
    outputBuffer[6] = objB(2.0, 3.0);           // ImplB: 2.0 * 3.0 = 6.0
    // REPORT-DAG: ([[# @LINE-1]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
    // CHECK: 6.0

    // Regular method to verify __init ran correctly
    outputBuffer[7] = float(objB.getValue());   // ImplB: _val = 20
    // REPORT-DAG: ([[# @LINE-1]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
    // CHECK: 20.0
}
