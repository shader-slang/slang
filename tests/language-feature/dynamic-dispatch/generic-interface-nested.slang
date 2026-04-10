// Nested generic interfaces where both levels use dynamic dispatch.
// ITransform<T> receives an IScaler<T> parameter, and both the
// transform and scaler implementations are resolved at runtime.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-dx12 -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-mtl -compute -shaderobj -output-using-type
//TEST:SIMPLE(filecheck=REPORT):-target hlsl -stage compute -entry computeMain -report-dynamic-dispatch-sites

interface IScaler<T : IArithmetic>
{
    T scale(T input, T factor);
}

interface ITransform<T : IArithmetic>
{
    T apply(IScaler<T> scaler, T input);
}

struct MulScaler : IScaler<float>
{
    float scale(float input, float factor) { return input * factor; }
}

struct AddScaler : IScaler<float>
{
    float scale(float input, float factor) { return input + factor; }
}

struct SingleApply : ITransform<float>
{
    float apply(IScaler<float> scaler, float input)
    {
        return scaler.scale(input, 2.0);
        // REPORT: {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
    }
}

struct DoubleApply : ITransform<float>
{
    float apply(IScaler<float> scaler, float input)
    {
        return scaler.scale(scaler.scale(input, 2.0), 3.0);
        // REPORT: {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
        // REPORT: {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
    }
}

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(int3 tid : SV_DispatchThreadID)
{
    int id = tid.x;

    IScaler<float> scaler;
    if (id == 0)
        scaler = MulScaler();
    else
        scaler = AddScaler();

    ITransform<float> xform;
    if (id == 0)
        xform = SingleApply();
    else
        xform = DoubleApply();

    // SingleApply + MulScaler: 5.0 * 2.0 = 10.0
    outputBuffer[0] = xform.apply(scaler, 5.0);
    // CHECK: 10.0
    // REPORT: {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:

    // Test with other combination: DoubleApply + AddScaler
    ITransform<float> xform2 = DoubleApply();
    IScaler<float> scaler2 = AddScaler();
    // DoubleApply + AddScaler: (5.0 + 2.0) + 3.0 = 10.0
    outputBuffer[1] = xform2.apply(scaler2, 5.0);
    // CHECK: 10.0

    // DoubleApply + MulScaler: (5.0 * 2.0) * 3.0 = 30.0
    IScaler<float> scaler3 = MulScaler();
    outputBuffer[2] = xform2.apply(scaler3, 5.0);
    // CHECK: 30.0
}
