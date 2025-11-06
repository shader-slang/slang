//TEST:SIMPLE(filecheck=CUDA): -target cuda -line-directive-mode none

// Verify that struct parameters in [CudaDeviceExport] functions are passed by value,
// not transformed to pointers by the copy-elision pass.
// This is a regression test for issue #8874.

struct CommonParameters
{
    int id;
    float value;
}

struct Parameters
{
    float3 position;
    float scale;
}

// CUDA: __device__ Parameters_[[#]] processParameters(uint idx_[[#]], CommonParameters_[[#]] commonParams_[[#]])
[CudaDeviceExport]
Parameters processParameters(uint idx, CommonParameters commonParams)
{
    Parameters result;
    result.position = float3(commonParams.value, commonParams.id, 0.0f);
    result.scale = commonParams.value + float(idx);
    return result;
}
