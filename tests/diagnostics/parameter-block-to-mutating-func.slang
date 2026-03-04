//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv
struct Data {
    StructuredBuffer<float> input[2];
    RWStructuredBuffer<float> output;
    uint input_tensor_count;
    StructuredBuffer<uint> index_buffer;
    uint index_count;

    [mutating]
    float fetch(int buffer, int index)
    {
        return input[buffer][index];
    }
};

[shader("compute")]
[numthreads(8, 8, 1)]
void compute_main(uint3 tid: SV_DispatchThreadID, ParameterBlock<Data> data)
{
    float result = 0.0;
    for (int i = 0; i < data.index_count; ++i) {
        uint buffer = data.index_buffer[i];
        result += data.fetch(buffer, tid.x * 1024 + tid.y);
//CHECK:                    ^ mutating method cannot be called on immutable value
//CHECK:                    ^ mutating method 'fetch' cannot be called on an immutable value
    }
    data.output[tid.x * 1024 + tid.y] = result;
}
