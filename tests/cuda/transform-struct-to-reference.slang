//TEST:SIMPLE(filecheck=CUDA): -stage compute -entry computeMain -target cuda
struct Data {
    StructuredBuffer<float> input[2];
    RWStructuredBuffer<float> output;
    uint input_tensor_count;
    StructuredBuffer<uint> index_buffer;
    uint index_count;

    // CUDA:  float Data_fetch_0(const Data_0 * this_0, int buffer_0, int index_0)
    float fetch(int buffer, int index)
    {
        return input[buffer][index];
    }
};

ParameterBlock<Data> data;

[shader("compute")]
[numthreads(8, 8, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    float result = 0.0;
    for (int i = 0; i < data.index_count; ++i) {
        uint buffer = data.index_buffer[i];
        result += data.fetch(buffer, tid.x * 1024 + tid.y);
    }
    data.output[tid.x * 1024 + tid.y] = result;
}
