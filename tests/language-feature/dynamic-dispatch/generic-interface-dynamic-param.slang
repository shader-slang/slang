// Generic interfaces where the generic parameter T is statically known
// but the concrete implementing type is dynamically resolved. Tests
// multiple specializations of the same generic interface coexisting,
// generic functions forwarding through existential parameters, and
// nested generic interfaces with multi-level dynamic dispatch.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-dx12 -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-mtl -compute -shaderobj -output-using-type
//TEST:SIMPLE(filecheck=REPORT):-target hlsl -stage compute -entry computeMain -report-dynamic-dispatch-sites

interface IProcessor<T : IArithmetic>
{
    T process(T input);
}

struct FloatDoubler : IProcessor<float>
{
    float process(float input) { return input * 2.0; }
}

struct FloatNegator : IProcessor<float>
{
    float process(float input) { return -input; }
}

struct IntDoubler : IProcessor<int>
{
    int process(int input) { return input * 2; }
}

struct IntNegator : IProcessor<int>
{
    int process(int input) { return -input; }
}

// Generic function receiving an existential IProcessor<T>.
// T is statically bound per call; the concrete type is dispatched at runtime.
T applyProcessor<T : IArithmetic>(IProcessor<T> op, T input)
{
    return op.process(input);
    // REPORT: {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
}

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(int3 tid : SV_DispatchThreadID)
{
    int id = tid.x;

    // IProcessor<float> with two possible types
    IProcessor<float> floatOp;
    if (id == 0)
        floatOp = FloatDoubler();
    else
        floatOp = FloatNegator();

    outputBuffer[0] = floatOp.process(5.0);
    // CHECK: 10.0

    // IProcessor<int> with two possible types (independent dispatch)
    IProcessor<int> intOp;
    if (id == 0)
        intOp = IntDoubler();
    else
        intOp = IntNegator();

    outputBuffer[1] = float(intOp.process(7));
    // CHECK: 14.0

    // Generic function forwarding: T=float
    outputBuffer[2] = applyProcessor(floatOp, 3.0);
    // CHECK: 6.0

    // Generic function forwarding: T=int
    outputBuffer[3] = float(applyProcessor(intOp, 4));
    // CHECK: 8.0

    // Verify specialization: known concrete type avoids dispatch
    IProcessor<float> known = FloatDoubler();
    outputBuffer[4] = known.process(100.0);
    // CHECK: 200.0
}
