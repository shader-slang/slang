//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST:SIMPLE(filecheck=REPORT):-target hlsl -stage compute -entry computeMain -report-dynamic-dispatch-sites

// Test for GitHub issue #9887: Dynamic Dispatch of Special Members — Generic Subscript
// Verifies that a generic subscript operator __subscript<T>(T index) -> float works
// correctly through dynamic dispatch with multiple type arguments for T.
//
// The same generic subscript is called with T=int, T=float, and T=uint at different
// call sites, each generating its own dispatch function (similar to generic methods).

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

interface ILookup
{
    __subscript<T>(T index) -> float { get; }
}

// ImplA: returns _val + _bonus regardless of index
struct ImplA : ILookup
{
    float _val;
    float _bonus;

    __subscript<T>(T index) -> float
    {
        get { return _val + _bonus; }
    }
}

// ImplB: returns _val * _scale regardless of index
struct ImplB : ILookup
{
    float _val;
    float _scale;

    __subscript<T>(T index) -> float
    {
        get { return _val * _scale; }
    }
}

// ImplA(5.0, 2.0) -> subscript returns 5.0 + 2.0 = 7.0
// ImplB(3.0, 4.0) -> subscript returns 3.0 * 4.0 = 12.0
ILookup makeObj(int id)
{
    if (id == 0) { ImplA a; a._val = 5.0; a._bonus = 2.0; return a; }
    else { ImplB b; b._val = 3.0; b._scale = 4.0; return b; }
}

float testInt(int id)
{
    ILookup obj = makeObj(id);
    return obj[int(0)];   // T=int
    // REPORT-DAG: ([[# @LINE-1]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
}

float testFloat(int id)
{
    ILookup obj = makeObj(id);
    return obj[float(0.5)]; // T=float — separate dispatch function from T=int
    // REPORT-DAG: ([[# @LINE-1]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
}

float testUint(int id)
{
    ILookup obj = makeObj(id);
    return obj[uint(2)];  // T=uint — another separate dispatch function
    // REPORT-DAG: ([[# @LINE-1]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
}

[numthreads(1, 1, 1)]
void computeMain(int id : SV_DispatchThreadID)
{
    // ImplA (id=0): all subscript calls return 7.0
    outputBuffer[0] = testInt(id);    // CHECK: 7.0
    outputBuffer[1] = testFloat(id);  // CHECK: 7.0
    outputBuffer[2] = testUint(id);   // CHECK: 7.0

    // ImplB (id+1=1): all subscript calls return 12.0
    outputBuffer[3] = testInt(id + 1);    // CHECK: 12.0
    outputBuffer[4] = testFloat(id + 1);  // CHECK: 12.0
    outputBuffer[5] = testUint(id + 1);   // CHECK: 12.0
}
