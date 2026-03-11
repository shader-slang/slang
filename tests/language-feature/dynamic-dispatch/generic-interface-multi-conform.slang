// A struct can implement multiple specializations of the same generic
// interface. Each specialization participates independently in dynamic
// dispatch when used through the corresponding existential type.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-dx12 -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-mtl -compute -shaderobj -output-using-type
//TEST:SIMPLE(filecheck=REPORT):-target hlsl -stage compute -entry computeMain -report-dynamic-dispatch-sites

interface IProcessor<T : IArithmetic>
{
    T process(T input);
}

struct Universal : IProcessor<float>, IProcessor<int>
{
    float process(float input) { return input * 2.0; }
    int process(int input) { return input * 3; }
}

struct FloatOnly : IProcessor<float>
{
    float process(float input) { return -input; }
}

struct IntOnly : IProcessor<int>
{
    int process(int input) { return -input; }
}

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(int3 tid : SV_DispatchThreadID)
{
    int id = tid.x;

    // Universal as IProcessor<float>, dispatched against FloatOnly
    IProcessor<float> floatOp;
    if (id == 0)
        floatOp = Universal();
    else
        floatOp = FloatOnly();

    // Universal.process(float) returns 5*2 = 10
    outputBuffer[0] = floatOp.process(5.0);
    // CHECK: 10.0
    // REPORT: {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:

    // Universal as IProcessor<int>, dispatched against IntOnly
    IProcessor<int> intOp;
    if (id == 0)
        intOp = Universal();
    else
        intOp = IntOnly();

    // Universal.process(int) returns 5*3 = 15
    outputBuffer[1] = float(intOp.process(5));
    // CHECK: 15.0
    // REPORT: {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:

    // Verify the two specializations are independent: using Universal
    // through IProcessor<float> does not affect IProcessor<int>
    IProcessor<float> f = Universal();
    IProcessor<int> i = Universal();
    outputBuffer[2] = f.process(7.0);
    // CHECK: 14.0
    outputBuffer[3] = float(i.process(7));
    // CHECK: 21.0
}
