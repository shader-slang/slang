int use_undefined_value(int k)
{
    int x;
    x += k;
    return x;
}

__generic<T>
T generic_undefined_return()
{
    T x;
    return x;
}

struct DiffStruct : IDifferentiable
{
    Texture2D tex;
}

[BackwardDifferentiable]
float f(float x)
{
    return x;
}

struct Data
{
    float value;
};

struct NestedData
{
    Data data;
};

RWStructuredBuffer<Data> inputBuffer;
RWStructuredBuffer<Data> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint tid = dispatchThreadID.x;

    Data inputData = inputBuffer[tid];
    Data undefData;
    NestedData nestedData;

    float uninitializedVariable;

    // CHECK: warning
    float outputData = inputData.value;
    outputData += uninitializedVariable;
    outputData += undefData.value;
    outputData += nestedData.data.value;

    var dp = diffPair(1.0);
    __bwd_diff(f)(dp, 1.0);
    outputData += dp.d.x;

    outputBuffer[tid].value = outputData;
}
