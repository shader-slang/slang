// Generic interface inheritance: IDerived<T> extends IBase<T>. Methods
// inherited from the base interface are dispatched correctly through
// the derived interface's existential type.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-dx12 -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-mtl -compute -shaderobj -output-using-type
//TEST:SIMPLE(filecheck=REPORT):-target hlsl -stage compute -entry computeMain -report-dynamic-dispatch-sites

interface IBase<T : IArithmetic>
{
    T getValue();
}

interface IDerived<T : IArithmetic> : IBase<T>
{
    T transform(T input);
}

struct Adder : IDerived<float>
{
    float v;
    float getValue() { return v; }
    float transform(float input) { return input + v; }
}

struct Scaler : IDerived<float>
{
    float v;
    float getValue() { return v * 2.0; }
    float transform(float input) { return input * v; }
}

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(int3 tid : SV_DispatchThreadID)
{
    int id = tid.x;

    IDerived<float> obj;
    if (id == 0)
        obj = Adder(10.0);
    else
        obj = Scaler(10.0);

    // Inherited method from IBase<float>, dispatched through IDerived<float>
    // Adder(10).getValue() = 10
    outputBuffer[0] = obj.getValue();
    // CHECK: 10.0
    // REPORT: {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:

    // Own method from IDerived<float>
    // Adder(10).transform(5) = 5 + 10 = 15
    outputBuffer[1] = obj.transform(5.0);
    // CHECK: 15.0
    // REPORT: {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:

    // Chain: inherited + own method
    // Adder(10).transform(Adder(10).getValue()) = transform(10) = 10 + 10 = 20
    outputBuffer[2] = obj.transform(obj.getValue());
    // CHECK: 20.0
}
