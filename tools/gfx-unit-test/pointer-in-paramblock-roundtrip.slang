struct PtrParams
{
    int* data;
    int scale;
};

ParameterBlock<PtrParams> params;
RWStructuredBuffer<int> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    output[0] = params.data[0] * params.scale + params.data[1];
}
