//TEST:SIMPLE(filecheck=CHECK_METAL): -target metal -entry computeMain -stage compute

// Regression test for global-varying builtins that are exposed by generic
// interface specialization after the early global-varying translation pass.

interface IWaveValue
{
    uint getValue();
}

struct WaveValue : IWaveValue
{
    uint getValue()
    {
        return WaveGetLaneIndex() + WaveGetWaveIndex() * 32;
    }
}

uint getGenericWaveValue<T: IWaveValue>(T value)
{
    return value.getValue();
}

RWStructuredBuffer<uint> outputBuffer;

// CHECK_METAL-DAG: thread_index_in_simdgroup
// CHECK_METAL-DAG: simdgroup_index_in_threadgroup

[shader("compute")]
[numthreads(64, 1, 1)]
void computeMain(uint tid : SV_GroupIndex)
{
    WaveValue value;
    outputBuffer[tid] = getGenericWaveValue<WaveValue>(value);
}
