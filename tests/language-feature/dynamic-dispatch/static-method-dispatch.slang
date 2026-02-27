// Static methods on interfaces are included in witness tables and can be
// dispatched dynamically when the concrete type is not known at compile time.
// This test exercises calling static methods directly on interface-typed
// values (via `obj.staticMethod()` syntax) and verifies that the compiler
// generates correct dynamic dispatch code that selects the right implementation
// at runtime.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-dx12 -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-mtl -compute -shaderobj -output-using-type
//TEST:SIMPLE(filecheck=REPORT):-target hlsl -stage compute -entry computeMain -report-dynamic-dispatch-sites

interface ICompute
{
    static float defaultValue();
    static int typeTag();
    float transform(float x);
}

struct AddOp : ICompute
{
    float bias;
    static float defaultValue() { return 1.0; }
    static int typeTag() { return 100; }
    float transform(float x) { return x + bias; }
}

struct MulOp : ICompute
{
    float scale;
    static float defaultValue() { return 2.0; }
    static int typeTag() { return 200; }
    float transform(float x) { return x * scale; }
}

// Accepts an interface-typed parameter and calls both static and instance
// methods, triggering dynamic dispatch for both kinds.
float processStatic(ICompute op)
{
    return op.defaultValue();
    // REPORT: {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
}

int getTag(ICompute op)
{
    return op.typeTag();
    // REPORT: {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
}

float processInstance(ICompute op, float x)
{
    return op.transform(x);
    // REPORT: {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
}

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    ICompute op;

    // Flow two types into the same interface variable so the compiler
    // cannot statically resolve the dispatch.
    if (outputBuffer[0] == 0.0)
        op = AddOp(10.0);
    else
        op = MulOp(3.0);

    // AddOp path (outputBuffer[0] starts at 0):
    // defaultValue() -> 1.0, transform(1.0) -> 1.0 + 10.0 = 11.0
    float base = processStatic(op);
    float result = processInstance(op, base);
    outputBuffer[0] = base;         // CHECK: 1.0
    outputBuffer[1] = result;       // CHECK: 11.0
    outputBuffer[2] = float(getTag(op)); // CHECK: 100.0

    // Reassign to a known type. The compiler may still specialize this,
    // but the values should be correct either way.
    ICompute op2 = MulOp(5.0);
    float base2 = processStatic(op2);
    float result2 = processInstance(op2, base2);
    outputBuffer[3] = base2;        // CHECK: 2.0
    outputBuffer[4] = result2;      // CHECK: 10.0
    outputBuffer[5] = float(getTag(op2)); // CHECK: 200.0
}
