//TEST:SIMPLE(filecheck=CUDA): -stage compute -entry computeMain -target cuda -line-directive-mode none
//TEST:SIMPLE(filecheck=PTX): -stage compute -entry computeMain -target cuda

struct Data {
    StructuredBuffer<float> input[2];
    RWStructuredBuffer<float> output;
    uint input_tensor_count;
    StructuredBuffer<uint> index_buffer;
    uint index_count;

    // CUDA: __device__ float Data_fetch{{.*}}(int {{.*}}, int {{.*}})
    // CUDA-NEXT: {
    // CUDA-NEXT: globalParams{{.*}}->data{{.*}}->input{{.*}}[{{.*}}]
    float fetch(int buffer, int index)
    {
        return input[buffer][index];
    }
};

ParameterBlock<Data> data;

// PTX: computeMain

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
