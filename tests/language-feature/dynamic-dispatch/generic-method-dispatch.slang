// Test: Generic methods in interfaces called through dynamic dispatch.
// Verifies that method<T>() specializations dispatch correctly at runtime
// when the concrete interface type is not known at compile time.
// Tests the same generic method called with multiple type arguments (float and int).
// See also: generic-method.slang (constrained with __BuiltinFloatingPointType), issue #9886

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST:SIMPLE(filecheck=REPORT):-target hlsl -stage compute -entry computeMain -report-dynamic-dispatch-sites

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

interface ISerializer
{
    // Unconstrained generic method: no type bound on T
    T identity<T>(T value);

    // Constrained generic method with arithmetic type bound
    T doubled<T : __BuiltinArithmeticType>(T value);

    // Void generic method: completely unconstrained, no return value
    void process<T>(T value);
}

struct IdentitySerializer : ISerializer
{
    T identity<T>(T value) { return value; }
    T doubled<T : __BuiltinArithmeticType>(T value) { return value + value; }
    void process<T>(T value) { }
}

struct NegatingSerializer : ISerializer
{
    T identity<T>(T value) { return value; }
    T doubled<T : __BuiltinArithmeticType>(T value) { return -(value + value); }
    void process<T>(T value) { }
}

ISerializer makeSerializer(int id)
{
    if (id == 0) return IdentitySerializer();
    else return NegatingSerializer();
}

// Test: same generic method called with float argument; 2 possible dispatch targets
float testDoubledFloat(int id, float x)
{
    ISerializer s = makeSerializer(id);
    return s.doubled<float>(x);
    // REPORT-DAG: ([[# @LINE-1]]): {{.*}} generated {{.*}} dynamic dispatch code{{.*}} 2 possible types:
}

// Test: same generic method called with int argument; 2 possible dispatch targets
int testDoubledInt(int id, int x)
{
    ISerializer s = makeSerializer(id);
    return s.doubled<int>(x);
    // REPORT-DAG: ([[# @LINE-1]]): {{.*}} generated {{.*}} dynamic dispatch code{{.*}} 2 possible types:
}

// Test: unconstrained identity<T> called with float
float testIdentityFloat(int id, float x)
{
    ISerializer s = makeSerializer(id);
    return s.identity<float>(x);
    // REPORT-DAG: ([[# @LINE-1]]): {{.*}} generated {{.*}} dynamic dispatch code{{.*}} 2 possible types:
}

// Test: unconstrained identity<T> called with int
int testIdentityInt(int id, int x)
{
    ISerializer s = makeSerializer(id);
    return s.identity<int>(x);
    // REPORT-DAG: ([[# @LINE-1]]): {{.*}} generated {{.*}} dynamic dispatch code{{.*}} 2 possible types:
}

[numthreads(1, 1, 1)]
void computeMain()
{
    // doubled<float>: IdentitySerializer returns x+x, NegatingSerializer returns -(x+x)
    outputBuffer[0] = testDoubledFloat(0, 3.0);   // CHECK: 6
    outputBuffer[1] = testDoubledFloat(1, 3.0);   // CHECK: -6

    // doubled<int>: same method, different type argument — dispatch to same 2 types
    outputBuffer[2] = float(testDoubledInt(0, 5)); // CHECK: 10
    outputBuffer[3] = float(testDoubledInt(1, 5)); // CHECK: -10

    // identity<float>: unconstrained generic method, both implementations return value as-is
    outputBuffer[4] = testIdentityFloat(0, 7.0);  // CHECK: 7
    outputBuffer[5] = testIdentityFloat(1, 7.0);  // CHECK: 7

    // identity<int>: same method, different type argument
    outputBuffer[6] = float(testIdentityInt(0, 9)); // CHECK: 9
    outputBuffer[7] = float(testIdentityInt(1, 9)); // CHECK: 9

    // process<T> is void — verify it compiles and can be called through dynamic dispatch
    ISerializer s = makeSerializer(0);
    s.process<float>(1.0);
    s.process<int>(42);
}
