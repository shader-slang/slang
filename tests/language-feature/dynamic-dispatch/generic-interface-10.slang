// Nested generic interfaces: a generic interface whose method returns
// another generic interface type. Tests that dynamic dispatch works
// when generic interfaces are composed.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST:SIMPLE(filecheck=REPORT):-target hlsl -stage compute -entry computeMain -report-dynamic-dispatch-sites

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

interface IValue<T>
{
    T get();
}

struct FloatConst : IValue<float>
{
    float val;
    float get() { return val; }
}

struct FloatDouble : IValue<float>
{
    float val;
    float get() { return val * 2.0; }
}

IValue<float> makeValue(int id, float v)
{
    if (id == 0) return FloatConst(v);
    else return FloatDouble(v);
}

interface IProcessor<T>
{
    T process(IValue<T> input);
}

struct AddOneProcessor : IProcessor<float>
{
    float process(IValue<float> input) { return input.get() + 1.0; }
}

struct SquareProcessor : IProcessor<float>
{
    float process(IValue<float> input) { return input.get() * input.get(); }
}

IProcessor<float> makeProcessor(int id)
{
    if (id == 0) return AddOneProcessor();
    else return SquareProcessor();
}

float test(int procId, int valId, float v)
{
    let proc = makeProcessor(procId);
    let val = makeValue(valId, v);
    return proc.process(val);
    // REPORT: ([[# @LINE-1]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
}

[numthreads(1, 1, 1)]
void computeMain()
{
    // AddOneProcessor + FloatConst(5.0): 5.0 + 1.0 = 6.0
    outputBuffer[0] = test(0, 0, 5.0); // CHECK: 6.0
    // AddOneProcessor + FloatDouble(5.0): 10.0 + 1.0 = 11.0
    outputBuffer[1] = test(0, 1, 5.0); // CHECK: 11.0
    // SquareProcessor + FloatConst(3.0): 3.0 * 3.0 = 9.0
    outputBuffer[2] = test(1, 0, 3.0); // CHECK: 9.0
    // SquareProcessor + FloatDouble(3.0): 6.0 * 6.0 = 36.0
    outputBuffer[3] = test(1, 1, 3.0); // CHECK: 36.0
}
